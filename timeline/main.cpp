//#include "communicator.h"
#include "mainwindow.h"
#include "taskmanager.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    const qint64 roleId1 = TaskManager::instance()->addRole(QSharedPointer<Role>(new Role("role 1")));
    const qint64 roleId2 = TaskManager::instance()->addRole(QSharedPointer<Role>(new Role("role 2")));

    const qint64 taskId1 = TaskManager::instance()->addTask(QSharedPointer<Task>(new Task("task 1")));
    const qint64 taskId2 = TaskManager::instance()->addTask(QSharedPointer<Task>(new Task("task 2")));
    const qint64 taskId3 = TaskManager::instance()->addTask(QSharedPointer<Task>(new Task("task 3")));
    const qint64 taskId4 = TaskManager::instance()->addTask(QSharedPointer<Task>(new Task("task 4")));

    TaskManager::instance()->assignTaskToRole(taskId1, roleId2);
    TaskManager::instance()->assignTaskToRole(taskId2, roleId2);
    TaskManager::instance()->assignTaskToRole(taskId4, roleId2);
    TaskManager::instance()->assignTaskToRole(taskId1, roleId1); // to check reassignment to a different role

    MainWindow window;
    window.show();

    return app.exec();
}
