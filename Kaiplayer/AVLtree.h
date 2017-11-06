//  Copyright (c) 2017, £ukasz G¹sowski

//  Kainote is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  Kainote is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with Kainote.  If not, see <http://www.gnu.org/licenses/>.

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
		int deleteItemByNode(int key, Node *node, bool addIndex);
        int height(Node*);
        int getBalance(Node*);

        void rightToLeftOrder(Node*, const func&);
        void leftToRightOrder(Node*, const func&);
        void fromTo(Node*, int from, int to, const func&);
        void fromToKey(Node*, int from, int to, const func&);


        class func{
            public:
				virtual void operator()(Node *node) const{};
				virtual void operator()(Node *node, int i) const{};
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

		void insert(int key, bool moveKeys = true);
        int getElementByKey(int key);
        int getElementById(int id);
		int deleteItemByKey(int key, bool moveKeys = true);
		int  deleteItemById(int id, bool moveKeys = true);
        void leftToRightOrder(const func&);
        void fromTo(int from, int to, const func&);
        void fromToKey(int from, int to, const func&);

        void addIndex(int i, int from, Node *root );
		int size();


        int operator[](const int key);

    protected:


};

#endif // AVLTREE_H
