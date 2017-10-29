#ifndef AVLTREE_H
#define AVLTREE_H

class AVLtree
{
    class func;
    private:
        class Node{
            public:
                int key;
                int num_of_elements_on_left;
                int num_of_elements_on_right;
                Node *left;
                Node *right;
                Node *previous;
                int height;

                Node();
                Node(int key, Node *previous);
                ~Node();
        };

        bool tt = false;
        int id = 0;

        Node *previous;

        Node *repairNumOf(Node*);
        Node *insert(Node *node, int key);
        Node *getElementByKey(Node *node, int key);
        Node *getElementById(Node *node, int id);
        Node *getNext(Node *right_node);
        Node *rightRotate(Node*);
        Node *leftRotate(Node*);
        int height(Node*);
        int getBalance(Node*);

        void preOrder(Node*,const func&);
        void rightToLeftOrder(Node*, const func&);
        void leftToRightOrder(Node*, const func&);
        void fromTo(Node*, int from, int to, const func&);
        void fromToKey(Node*, int from, int to, const func&);


        class func{
            public:
            virtual void operator()(Node *node) const;
            virtual void operator()(Node *node, int i) const;
        };

        class changeIndex : public func{
            void operator()(Node *node, int i) const {

            }
            void operator()(Node *node) const {}
        };

        class print : public func{
            void operator()(Node *node, int i) const {}
            void operator()(Node *node) const {
                

            }
        };

        class del : public func{
            void operator()(Node *node, int i) const {}
            void operator()(Node *node) const {
                delete node;
            };
        };



    public:
        AVLtree();
        virtual ~AVLtree();

        Node *root;

        void insert(int key);
        int getElementByKey(int key);
        int getElementById(int id);
        int deleteItemByKey(int key);

        void printF();

        void preOrder(const func&);
        void leftToRightOrder(const func&);
        void fromTo(int from, int to, const func&);
        void fromToKey(int from, int to, const func&);

        void addIndex(int i, int from, Node *root );

        int operator[](const int key);

    protected:


};

#endif // AVLTREE_H
