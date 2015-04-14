//#include "communicator.h" // ### implement later to handle communication with other clients via central server
#include "mainwindow.h"
#include "taskmanager.h"
#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qsrand(QDateTime::currentDateTime().toTime_t());

    const qint64 roleId1 = TaskManager::instance()->addRole(QSharedPointer<Role>(new Role("role 1", QTime(8, 15), QTime(16, 0))));
    const qint64 roleId2 = TaskManager::instance()->addRole(QSharedPointer<Role>(new Role("role 2", QTime(8, 15), QTime(16, 0))));
    const qint64 roleId3 = TaskManager::instance()->addRole(QSharedPointer<Role>(new Role("role 3", QTime(8, 15), QTime(16, 0))));
    const qint64 roleId4 = TaskManager::instance()->addRole(QSharedPointer<Role>(new Role("role 4", QTime(8, 15), QTime(16, 0))));
    const qint64 roleId5 = TaskManager::instance()->addRole(QSharedPointer<Role>(new Role("role 5", QTime(8, 15), QTime(16, 0))));
    const qint64 roleId6 = TaskManager::instance()->addRole(QSharedPointer<Role>(new Role("role 6", QTime(8, 15), QTime(16, 0))));
    const qint64 roleId7 = TaskManager::instance()->addRole(QSharedPointer<Role>(new Role("role 7", QTime(8, 15), QTime(16, 0))));

    const qint64 taskId1 = TaskManager::instance()
            ->addTask(QSharedPointer<Task>(new Task("task 1",
                                                    QDateTime(QDate(2015, 4, 1), QTime(0, 0)),
                                                    QDateTime(QDate(2015, 4, 2), QTime(0, 0)))));
    const qint64 taskId2 = TaskManager::instance()
            ->addTask(QSharedPointer<Task>(new Task("task 2",
                                                    QDateTime(QDate(2015, 4, 1), QTime(12, 0)),
                                                    QDateTime(QDate(2015, 4, 2), QTime(0, 0)))));
    const qint64 taskId3 = TaskManager::instance()
            ->addTask(QSharedPointer<Task>(new Task("task 3",
                                                    QDateTime(QDate(2015, 4, 1), QTime(0, 0)),
                                                    QDateTime(QDate(2015, 4, 1), QTime(12, 0)))));
    const qint64 taskId4 = TaskManager::instance()
            ->addTask(QSharedPointer<Task>(new Task("task 4",
                                                    QDateTime(QDate(2015, 4, 1), QTime(23, 45)),
                                                    QDateTime(QDate(2015, 4, 2), QTime(0, 15)))));

    const qint64 taskId5 = TaskManager::instance()
            ->addTask(QSharedPointer<Task>(new Task("task 5",
                                                    QDateTime(QDate(2015, 4, 2), QTime(0, 0)),
                                                    QDateTime(QDate(2015, 4, 2), QTime(1, 0)))));

    TaskManager::instance()->assignTaskToRole(taskId1, roleId2);
    TaskManager::instance()->assignTaskToRole(taskId2, roleId2);
    TaskManager::instance()->assignTaskToRole(taskId4, roleId3);
    TaskManager::instance()->assignTaskToRole(taskId1, roleId1); // to check reassignment to a different role
    TaskManager::instance()->assignTaskToRole(taskId3, roleId4);
    TaskManager::instance()->assignTaskToRole(taskId5, roleId4);

    MainWindow window(QDate(2015, 4, 1), 7);
    window.show();

    TaskManager::instance()->emitUpdated(); // ensure views are initialized
    return app.exec();
}
