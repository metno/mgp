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

    const qint64 roleId1 = TaskManager::instance().addRole(RoleProperties("role 1", QTime(8, 15), QTime(16, 0)));
    const qint64 roleId2 = TaskManager::instance().addRole(RoleProperties("role 2", QTime(15, 0), QTime(23, 0)));
    const qint64 roleId3 = TaskManager::instance().addRole(RoleProperties("role 3", QTime(15, 0), QTime(23, 0)));
    const qint64 roleId4 = TaskManager::instance().addRole(RoleProperties("role 4", QTime(8, 15), QTime(16, 0)));
    const qint64 roleId5 = TaskManager::instance().addRole(RoleProperties("role 5", QTime(8, 15), QTime(16, 0)));
    const qint64 roleId6 = TaskManager::instance().addRole(RoleProperties("role 6", QTime(22, 0), QTime(6, 0)));
    const qint64 roleId7 = TaskManager::instance().addRole(RoleProperties("role 7", QTime(8, 15), QTime(16, 0)));

    const qint64 taskId1 = TaskManager::instance().addTask(
                TaskProperties(
                    "task 1", "<summary for task 1>", "description of task 1<br/>another line",
                    QDateTime(QDate(2015, 4, 1), QTime(0, 0)),
                    QDateTime(QDate(2015, 4, 2), QTime(0, 0))));
    const qint64 taskId2 = TaskManager::instance().addTask(
                TaskProperties(
                    "task 2", "<summary for task 2>", "description of task 2<br/>another line",
                    QDateTime(QDate(2015, 4, 1), QTime(12, 0)),
                    QDateTime(QDate(2015, 4, 2), QTime(0, 0))));
    const qint64 taskId3 = TaskManager::instance().addTask(
                TaskProperties(
                    "task 3", "<summary for task 3>", "description of task 3<br/>another line",
                    QDateTime(QDate(2015, 4, 1), QTime(0, 0)),
                    QDateTime(QDate(2015, 4, 1), QTime(12, 0))));
    const qint64 taskId4 = TaskManager::instance().addTask(
                TaskProperties(
                    "task 4", "<summary for task 4>", "description of task 4<br/>another line",
                    QDateTime(QDate(2015, 4, 1), QTime(23, 45)),
                    QDateTime(QDate(2015, 4, 2), QTime(0, 15))));

    const qint64 taskId5 = TaskManager::instance().addTask(
                TaskProperties(
                    "task 5", "<summary for task 5>", "description of task 5<br/>another line",
                    QDateTime(QDate(2015, 4, 2), QTime(0, 0)),
                    QDateTime(QDate(2015, 4, 2), QTime(1, 0))));

    TaskManager::instance().assignTaskToRole(taskId1, roleId2);
    TaskManager::instance().assignTaskToRole(taskId2, roleId2);
    TaskManager::instance().assignTaskToRole(taskId4, roleId3);
    TaskManager::instance().assignTaskToRole(taskId1, roleId1); // to check reassignment to a different role
    TaskManager::instance().assignTaskToRole(taskId3, roleId4);
    TaskManager::instance().assignTaskToRole(taskId5, roleId4);

    MainWindow::init(QDate(2015, 4, 1), 7);
    MainWindow::instance().show();

    TaskManager::instance().emitUpdated(); // ensure views are initialized
    return app.exec();
}
