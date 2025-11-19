#ifndef PARTICIPANT_MODEL_H
#define PARTICIPANT_MODEL_H

#include <QString>
#include <QAbstractListModel>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QListView>
#include <QSortFilterProxyModel>
#include <QDialog>
#include <QStandardItemModel>

struct Person {
    int userId;
    QString surName;
    QString name;
    QString fatherName; // optional


    QString displayName() const {
        // "Last First Middle" (omit empty parts)
        QStringList parts;
        if (!surName.isEmpty()) parts << surName;
        if (!name.isEmpty()) parts << name;
        if (!fatherName.isEmpty()) parts << fatherName;
        return parts.join(' ');
    }
};

class ParticipantsModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit ParticipantsModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void addParticipant(const Person &p);
    void removeParticipantAt(int row);
    QList<Person> participants() const { return m_parts; }
    void reset();

signals:
    void participantsChanged();

private:
    QList<Person> m_parts;
};

class PersonDialog : public QDialog {
    Q_OBJECT
public:
    explicit PersonDialog(QWidget* parent = nullptr);


    void setPerson(const Person &p);
    Person person() const;

private slots:
    void onAccept();


private:
    QLineEdit* firstEdit;
    QLineEdit* lastEdit;
    QLineEdit* middleEdit;
    QPushButton* okButton;
    QPushButton* cancelButton;
};

class ParticipantSelectorWidget : public QWidget {
    Q_OBJECT
public:
    explicit ParticipantSelectorWidget(QWidget* parent = nullptr);

    QList<Person> selectedParticipants() const;

    void update(int eventid);

signals:
    void participantsChanged(const QList<Person>& list);

private slots:
    void onAddNew();
    void onPeopleActivated(const QModelIndex &index);
    void onParticipantActivated(const QModelIndex &index);
    void onParticipantsChanged();

private:
    ParticipantsModel* m_participantsModel;

    int eventId = 0;

    QLineEdit* m_search;
    QListView* m_peopleView;
    QListView* m_participantsView;
    QSortFilterProxyModel* m_proxy;
    QPushButton* m_addNewBtn;
    QStandardItemModel* m_peopleModel;
};



#endif
