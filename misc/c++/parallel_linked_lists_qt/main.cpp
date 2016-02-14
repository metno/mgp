#include <QCoreApplication>
#include <QLinkedList>
#include <QLinkedListIterator>
#include <QString>
#include <QDebug>
#include <iostream>

struct Node
{
    QString tag_;
    bool forward_;
    QLinkedList<Node>::const_iterator neighbour_; // pointer to neighbour node in other list
    Node(const QString &tag, bool forward = true, const QLinkedList<Node>::const_iterator neighbour = QLinkedList<Node>::const_iterator())
        : tag_(tag)
        , forward_(forward)
        , neighbour_(neighbour)
    {}
};

static void printList(const QLinkedList<Node> &list, const QString &text)
{
    std::cout << QString("%1:").arg(text).toLatin1().data() << std::endl << "    ";
    QLinkedList<Node>::const_iterator it;
    for (it = list.constBegin(); it != list.constEnd(); ++it)
        std::cout << " " << it->tag_.toLatin1().data();
    std::cout << std::endl;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);


    // create lists a and b like this:
    //
    //   [A0] <----> [A1] <----> [A2] <----> [A3] <----> [A4]
    //
    //   [B0] <----> [B1] <----> [B2] <----> [B3] <----> [B4] <----> [B5] <----> [B6]
    QLinkedList<Node> a;
    for (int i = 0; i < 5; ++i)
        a.push_back(Node(QString("A%1").arg(i)));
    QLinkedList<Node> b;
    for (int i = 0; i < 7; ++i)
        b.push_back(Node(QString("B%1").arg(i)));
    printList(a, "initial a");
    printList(b, "initial b");


    // insert new nodes like like this:
    //
    //   [A0] <----> [A1] <----> [X0] <----> [A2] <----> [A3] <----> [A4]
    //
    //   [B0] <----> [B1] <----> [B2] <----> [B3] <----> [Y0] <----> [B4] <----> [B5] <----> [B6]
    {
        QLinkedList<Node>::iterator it = a.begin();
        it += 2;
        a.insert(it, Node("X0"));
        printList(a, "a after inserting X0");
    }
    {
        QLinkedList<Node>::iterator it = b.begin();
        it += 4;
        b.insert(it, Node("Y0"));
        printList(b, "b after inserting Y0");
    }


    // set up neighbour links like this:
    //
    //   [A0] <----> [A1] <----> [X0] <----> [A2] <===== [A3] <----> [A4]
    //                            ^                       ^
    //                            |                       |
    //                            +-----------+           |
    //                                        |           |
    //                                        v           v
    //   [B0] <----> [B1] <----> [B2] <----> [B3] =====> [Y0] <----> [B4] <----> [B5] <----> [B6]
    {
        // [X0] <----> [B3]
        QLinkedList<Node>::iterator a_it = a.begin();
        a_it += 2;
        Q_ASSERT(a_it->tag_ == "X0");
        QLinkedList<Node>::iterator b_it = b.begin();
        b_it += 3;
        Q_ASSERT(b_it->tag_ == "B3");
        a_it->neighbour_ = b_it;
        b_it->neighbour_ = a_it;

        b_it->forward_ = true;
    }
    {
        // [A3] <----> [Y0]
        QLinkedList<Node>::iterator a_it = a.begin();
        a_it += 4;
        Q_ASSERT(a_it->tag_ == "A3");
        QLinkedList<Node>::iterator b_it = b.begin();
        b_it += 4;
        Q_ASSERT(b_it->tag_ == "Y0");
        a_it->neighbour_ = b_it;
        b_it->neighbour_ = a_it;

        a_it->forward_ = false;
    }

    // find first cycle
    std::cout << "\ntraversing to find cycle:\n     ";
    {
        QLinkedList<Node>::const_iterator start;
        QLinkedList<Node>::const_iterator it = a.constBegin();
        bool forward = true;
        while (true)
        {
            std::cout << it->tag_.toLatin1().data() << " ";

            if (it == start) {
                std::cout << "(back at start again) ";
                break; // done
            }

            if (it->neighbour_ == QLinkedList<Node>::const_iterator()) {
                // not at a link to the other list; continue to traverse along the current list

                if (forward)
                    it++;
                else
                    it--;

            } else {
                // at a link to the other list; move to the other list and change direction
                if (start == QLinkedList<Node>::const_iterator()) {
                    std::cout << "(start) ";
                    start = it;
                }

                std::cout << "(move to other list) ";
                it = it->neighbour_;

                std::cout << it->tag_.toLatin1().data() << " ";

                forward = it->forward_;
                if (forward) {
                    it++;
                } else {
                    it--;
                }
            }
        }

        std::cout << std::endl;
    }


//    qDebug() << "uninitialized neighbour:" << (b.constBegin()->neighbour_ == QLinkedList<Node>::const_iterator());

    return 0;
}
