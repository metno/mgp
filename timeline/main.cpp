//#include "communicator.h" // ### implement later to handle communication with other clients via central server
#include "mainwindow.h"
#include "taskmanager.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    const qint64 roleId1 = TaskManager::instance()->addRole(QSharedPointer<Role>(new Role("role 1")));
    const qint64 roleId2 = TaskManager::instance()->addRole(QSharedPointer<Role>(new Role("role 2")));
    const qint64 roleId3 = TaskManager::instance()->addRole(QSharedPointer<Role>(new Role("role 3")));
    const qint64 roleId4 = TaskManager::instance()->addRole(QSharedPointer<Role>(new Role("role 4")));
    const qint64 roleId5 = TaskManager::instance()->addRole(QSharedPointer<Role>(new Role("role 5")));
    const qint64 roleId6 = TaskManager::instance()->addRole(QSharedPointer<Role>(new Role("role 6")));
    const qint64 roleId7 = TaskManager::instance()->addRole(QSharedPointer<Role>(new Role("role 7")));

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

    TaskManager::instance()->emitUpdated(); // ensure views are initialized
    return app.exec();
}
