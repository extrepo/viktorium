// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/common.h>
#include <spdlog/details/file_helper.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/os.h>
#include <spdlog/details/circular_q.h>
#include <spdlog/details/synchronous_factory.h>
#include <dirent.h>
//#include <unilog/unilogzip.h>
#include <regex>

#include <chrono>
#include <cstdio>
#include <ctime>
#include <mutex>
#include <string>
#include <vector>
#include <functional>

namespace spdlog {
namespace sinks {

/*
 * Generator of daily log file names in format basename.YYYY-MM-DD.ext
 */
struct rotate_daily_filename_calculator
{
    // Create filename for the form basename.YYYY-MM-DD
    static filename_t calc_filename(const filename_t &filename, const tm &now_tm)
    {
        filename_t basename, ext;
        std::tie(basename, ext) = details::file_helper::split_by_extension(filename);
        int ind = basename.rfind(details::os::folder_sep);
        return fmt::format(
            SPDLOG_FILENAME_T("{}{:04d}{:02d}{:02d}-{:02d}{:02d}{:02d}-{}{}"),
                    filename.substr(0, ind + 1), now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec, basename.substr(ind + 1), ext);
    }
};

/*
 * Rotating file sink based on date.
 * If truncate != false , the created file will be truncated.
 * If max_files > 0, retain only the last max_files and delete previous.
 */
template<typename Mutex, typename FileNameCalc = rotate_daily_filename_calculator>
class rotate_daily_file_sink final : public base_sink<Mutex>
{
public:
    // create daily file sink which rotates on given time
    rotate_daily_file_sink(filename_t base_filename, int rotation_hour, int rotation_minute, size_t max_size, bool truncate = false, uint16_t max_files = 0)
        : base_filename_(std::move(base_filename))
        , rotation_h_(rotation_hour)
        , rotation_m_(rotation_minute)
        , max_size_(max_size)
        , truncate_(truncate)
        , max_files_(max_files)
        , filenames_q_()
    {
        if (rotation_hour < 0 || rotation_hour > 23 || rotation_minute < 0 || rotation_minute > 59)
        {
            throw_spdlog_ex("rotate_daily_file_sink: Invalid rotation time in ctor");
        }
        auto now = log_clock::now();
        auto filename = FileNameCalc::calc_filename(base_filename_, now_tm(now));
        if (max_files_ > 0)
        {
            check_file(filename); // expensive. called only once
        }
        else filenames_q_.push_back(std::move(filename));
        rotation_tp_ = next_rotation_tp_();
    }

    filename_t filename()
    {
        std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
        return file_helper_.filename();
    }

protected:
    void sink_it_(const details::log_msg &msg) override
    {
        auto time = msg.time;
        bool should_rotate = time >= rotation_tp_;
        auto filename_o = file_helper_.filename();
        if (should_rotate)
        {
            auto filename = FileNameCalc::calc_filename(base_filename_, now_tm(time));
            file_helper_.open(filename, truncate_);
            rotation_tp_ = next_rotation_tp_();
        }
        else if(file_helper_.filename().empty())
        {
            file_helper_.open(filenames_q_.at(filenames_q_.size() - 1), truncate_);
            current_size_ = file_helper_.size(); // expensive. called only once
        }
        memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(msg, formatted);
        file_helper_.write(formatted);
        current_size_ += formatted.size();
        // Check for filesize rotation
        if (current_size_ > max_size_)
        {
            auto filename = FileNameCalc::calc_filename(base_filename_, now_tm(time));
            file_helper_.open(filename, truncate_);
            rotation_tp_ = next_rotation_tp_();
            current_size_ = formatted.size();
            delete_old_();
        }
        // Do the cleaning only at the end because it might throw on failure.
        else if (should_rotate && max_files_ > 0)
        {
            delete_old_();
        }
    }

    void flush_() override
    {
        file_helper_.flush();
    }

    static void rotate_compress(std::string src_name, std::string archive_name)
    {
//        compress(src_name, archive_name);
        details::os::remove_if_exists(src_name);
    }

private:
    std::vector<std::string> find_files(const std::string &path, /*std::string_view*/ std::string pattern) {
        std::vector<std::string> paths_string;
        std::regex rg(pattern.data());
        DIR *dp;
        struct dirent *dirp;
        if((dp = opendir(path.c_str())) == NULL)
        {
            throw_spdlog_ex("Error opening dir: " + path, errno);
        }
        while((dirp = readdir(dp)) != NULL)
        {
            std::string cur_file = dirp->d_name;
            if(cur_file != "." && cur_file != ".." && std::regex_search(cur_file, rg))
            {
                paths_string.push_back(path + cur_file);
            }
        }
        closedir(dp);
        return paths_string;
    }

    /**
     * @brief
     * Function for checking exist files in current path
     * with removing log files according to parameters
     * @return true/false - result of checking
     */
    bool check_file(filename_t filename)
    {
        filenames_q_ = details::circular_q<filename_t>(static_cast<size_t>(max_files_));
        // First step - find all files according to format
        int ind = base_filename_.rfind(details::os::folder_sep);
        std::basic_string<char> short_name = details::os::filename_to_str(base_filename_).substr(ind + 1);
        std::basic_string<char> path = details::os::filename_to_str(base_filename_).substr(0, ind + 1);
        std::vector<std::string> all_matching_files;
        // First step - find filenames according to pattern
        all_matching_files = find_files(path, "[0-9]{4}[0-9]{2}[0-9]{2}-[0-9]{2}[0-9]{2}[0-9]{2}-" + short_name);
        // if no results - use name with current timestamp
        if(all_matching_files.empty())
        {
            filenames_q_.push_back(std::move(filename));
            return false;
        }
        // Second step - sorting it according to date and time
        int key = 0;
        for(unsigned int i = 0; i < all_matching_files.size() - 1; i++)
        {
            key = i + 1;
            std::string res = all_matching_files.at(key);
            for(int j = i + 1; j > 0; j--)
            {
                std::string res_j = all_matching_files.at(j - 1);
                if(!compareDate(res, res_j))
                {
                    all_matching_files.at(j) = all_matching_files.at(j - 1);
                    key = j - 1;
                }
            }
            all_matching_files.at(key) = res;
        }
        // Third step - record to memory valid filenames < max_files, remove old files
        for (unsigned int i = 0; i < all_matching_files.size(); i++)
        {
            if(i > all_matching_files.size() - 1 - max_files_ || all_matching_files.size() <= max_files_)
            {
                filenames_q_.push_back(std::move(all_matching_files.at(i)));
            }
            else details::os::remove_if_exists(all_matching_files.at(i));
//            else
//            {
//                if(archive_thread.joinable()) archive_thread.join();
//                std::string source = all_matching_files.at(i);
//                filename_t basename, ext;
//                std::tie(basename, ext) = details::file_helper::split_by_extension(source);
//                std::string dest = basename + ".arch";
//                archive_thread = std::thread(rotate_compress, std::move(source), std::move(dest));
//            }
        }
        // Final step - return end-valid filename or generate new filename
        tm date = now_tm(log_clock::now());
        date.tm_yday = date.tm_yday - 1;
        date.tm_hour = rotation_h_;
        date.tm_min = rotation_m_;
        date.tm_sec = 0;
        // test name for searching exist filenames before daily rotation
        auto test_filename = FileNameCalc::calc_filename(base_filename_, date);
        if(compareDate(filenames_q_.at(filenames_q_.size() - 1), test_filename))
        {
            // return exist file for opening
            return true;
        }
        // return filename with current timestamp
        else
        {
            if(filenames_q_.full()) filenames_q_.pop_front();
            filenames_q_.push_back(std::move(filename));
            return false;
        }
    }

    /**
     * @brief
     *  Function for generate time point in 'tm' format
     * @param tp - time point in log_clock format
     * @return
     *
     */
    tm now_tm(log_clock::time_point tp)
    {
        time_t tnow = log_clock::to_time_t(tp);
        return spdlog::details::os::localtime(tnow);
    }

    log_clock::time_point next_rotation_tp_()
    {
        auto now = log_clock::now();
        tm date = now_tm(now);
        date.tm_hour = rotation_h_;
        date.tm_min = rotation_m_;
        date.tm_sec = 0;
        auto rotation_time = log_clock::from_time_t(std::mktime(&date));
        if (rotation_time > now)
        {
            return rotation_time;
        }
        return {rotation_time + std::chrono::hours(24)};
    }

    // Delete the file N rotations ago.
    // Throw spdlog_ex on failure to delete the old file.
    void delete_old_()
    {
        using details::os::filename_to_str;
        using details::os::remove_if_exists;

        filename_t current_file = file_helper_.filename();
        if (filenames_q_.full())
        {
            auto old_filename = std::move(filenames_q_.front());
            filenames_q_.pop_front();
            bool ok = remove_if_exists(old_filename) == 0;
            if (!ok)
            {
                filenames_q_.push_back(std::move(current_file));
                throw_spdlog_ex("Failed removing daily file " + filename_to_str(old_filename), errno);
            }
        }
        filenames_q_.push_back(std::move(current_file));
    }

    bool compareDate(std::string first_file, std::string sec_file)
    {
        int ind = first_file.rfind(details::os::folder_sep);
        std::basic_string<char> short_name = details::os::filename_to_str(first_file).substr(ind + 1);
        int day_first = atoi(short_name.substr(0, short_name.find_first_of("-")).c_str());
        int time_first = atoi(short_name.substr(short_name.find_first_of("-") + 1).c_str());
        //
        ind = sec_file.rfind(details::os::folder_sep);
        short_name = details::os::filename_to_str(sec_file).substr(ind + 1);
        int day_sec = atoi(short_name.substr(0, short_name.find_first_of("-")).c_str());
        int time_sec = atoi(short_name.substr(short_name.find_first_of("-") + 1).c_str());
        if(day_sec == day_first)
        {
            return time_sec < time_first;
        }
        return day_sec < day_first;
    }

    filename_t base_filename_;
    int rotation_h_;
    int rotation_m_;
    std::size_t max_size_;
    log_clock::time_point rotation_tp_;
    details::file_helper file_helper_;
    bool truncate_;
    uint16_t max_files_;
    std::size_t current_size_;
    details::circular_q<filename_t> filenames_q_;
};

using rotate_daily_file_sink_mt = rotate_daily_file_sink<std::mutex>;
using rotate_daily_file_sink_st = rotate_daily_file_sink<details::null_mutex>;

} // namespace sinks

//
// factory functions
//
template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> rotate_daily_logger_mt(
    const std::string &logger_name, const filename_t &filename, int hour = 0, int minute = 0, size_t max_file_size = 0, bool truncate = false, uint16_t max_files = 0)
{
    return Factory::template create<sinks::rotate_daily_file_sink_mt>(logger_name, filename, hour, minute, max_file_size, truncate, max_files);
}

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> rotate_daily_logger_st(
    const std::string &logger_name, const filename_t &filename, int hour = 0, int minute = 0, size_t max_file_size = 0, bool truncate = false, uint16_t max_files = 0)
{
    return Factory::template create<sinks::rotate_daily_file_sink_st>(logger_name, filename, hour, minute, max_file_size, truncate, max_files);
}
} // namespace spdlog
