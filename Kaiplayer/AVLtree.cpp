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

#include "AVLtree.h"
#include <iostream>
#include "vld.h"

AVLtree::AVLtree()
{

}

AVLtree::~AVLtree()
{
    del Del;
    rightToLeftOrder(root,Del);
    delete root;
    root=NULL;
}

int AVLtree::operator[](int id){
   return getElementById(id);
}

int AVLtree::size()
{
	return root->num_of_elements_on_left + root->num_of_elements_on_right + 1;
}

///
/// Funkcje
///


int AVLtree::getElementByKey(int id){
    Node *t = getElementByKey(root, id);
    if(t!=NULL) return t->key;
    else return -1;
}

AVLtree::Node *AVLtree::getElementByKey(Node *root, int key){
	if (root == NULL) return NULL;
	if (key < root->key)
		return getElementByKey(root->left, key);
	else if (key > root->key)
		return getElementByKey(root->right, key);
	else if (key == root->key)
		return root;
	return NULL;
}

int AVLtree::getElementById(int id){
    Node *t = getElementById(root, id);
    if(t!=NULL) return t->key;
    else return -1;
}

AVLtree::Node *AVLtree::getElementById(Node *node, int id){
	if (node == NULL) return NULL;
	else if (id < node->num_of_elements_on_left)
		return getElementById(node->left, id);
	else if (id - node->num_of_elements_on_left <= 0)
		return node;
	else return getElementById(node->right, id - node->num_of_elements_on_left - 1);
}

void AVLtree::insert(int key){
    addIndex(1, key, root);
    if(root!=NULL)
        root=insert(root, key);
    else
        root=new Node(key, NULL);
	previous = NULL;
}


int AVLtree::height(Node *N)
{
	if (N == NULL)
		return 0;
	return N->height;
}

int max(int a, int b)
{
	return (a > b) ? a : b;
}


AVLtree::Node* AVLtree::insert(Node* node, int key)
{
	if (node == NULL){
		return new Node(key, previous);
	}

	if (key < node->key){
		previous = node;
		node->num_of_elements_on_left++;
		node->left = insert(node->left, key);
	}
	else if (key > node->key){
		previous = node;
		node->num_of_elements_on_right++;
		node->right = insert(node->right, key);
	}
	else 
		return node;

	node->height = 1 + max(height(node->left),
		height(node->right));

	int balance = getBalance(node);

	if (balance > 1 && key < node->left->key)
		return rightRotate(node);

	if (balance < -1 && key > node->right->key)
		return leftRotate(node);

	if (balance > 1 && key > node->left->key)
	{
		node->left = leftRotate(node->left);
		return rightRotate(node);
	}

	if (balance < -1 && key < node->right->key)
	{
		node->right = rightRotate(node->right);
		return leftRotate(node);
	}
	return node;
}


int AVLtree::deleteItemByKey(int key){
	Node *node=getElementByKey(root, key);

	if (node != NULL){
        int key = node->key;
		if (node->left == NULL && node->right == NULL){
			if (node->previous != NULL){
				if (node->previous->left == node){
					node->previous->left = NULL;
					--node->previous->num_of_elements_on_left;
				}
				else if (node->previous->right == node){
					node->previous->right = NULL;
					--node->previous->num_of_elements_on_right;
				}
				node->previous->height = 1 + max(height(node->previous->left),
                    height(node->previous->right));
				root = repairNumOf(node->previous);
			}
			if(node==root){
                delete root;
                root=NULL;
                node=NULL;
			} else {
                delete node;
                node = NULL;
			}

		}
		else if (node->left == NULL && node->right != NULL){
            if(node->previous!=NULL){
                if (node->previous->left == node){
                    node->previous->left = node->right;
                    node->right->previous = node->previous;
                    --node->previous->num_of_elements_on_left;
                }
                else{
                    node->previous->right = node->right;
                    node->right->previous = node->previous;
                    --node->previous->num_of_elements_on_right;
                }
                node->previous->height = 1 + max(height(node->previous->left),
                        height(node->previous->right));
                root = repairNumOf(node->previous);
            } else {
                root = node->right;
                root->previous=NULL;
            }
			delete node;
			node = NULL;
		}
		else if (node->left != NULL && node->right == NULL){
            if(node->previous!=NULL){
                if (node->previous->left == node){
                    node->previous->left = node->left;
                    node->left->previous = node->previous;
                    --node->previous->num_of_elements_on_left;
                }
                else{
                    node->previous->right = node->left;
                    node->left->previous = node->previous;
                    --node->previous->num_of_elements_on_right;
                }
                node->previous->height = 1 + max(height(node->previous->left),
                        height(node->previous->right));
                root = repairNumOf(node->previous);
            } else {
                root = node->left;
                root->previous=NULL;
            }
			delete node;
			node = NULL;
		}
		else {
			Node *temp = getNext(node->right);

			node->key = temp->key;

            if (temp->previous->left == temp){
                temp->previous->left = temp->right;
                --temp->previous->num_of_elements_on_left;
            }
            else {
                temp->previous->right = temp->right;
                --temp->previous->num_of_elements_on_right;
            }

			if (temp->right != NULL){
				temp->right->previous = temp->previous;
			}
			temp->previous->height = 1 + max(height(temp->previous->left),
                    height(temp->previous->right));
			root = repairNumOf(temp->previous);
			delete temp;
			temp = NULL;

		}
		addIndex(-1, key, root);
		return key;
	}
	return -1;
}

AVLtree::Node *AVLtree::getNext(Node *right_node){
    if(right_node!=NULL){
        if (right_node->left != NULL)
            return getNext(right_node->left);
        return right_node;
    }
    return NULL;
}

AVLtree::Node *AVLtree::repairNumOf(Node *node){

    int balance = getBalance(node);

    if (balance > 1 && node->left->left != NULL)
		node = rightRotate(node);

	else if (balance < -1 && node->right->right != NULL)
		node = leftRotate(node);

	else if (balance > 1 && node->left->right !=NULL)
	{
		node->left = leftRotate(node->left);
		node = rightRotate(node);
	}

	else if (balance < -1 && node->right->left != NULL)
	{
		node->right = rightRotate(node->right);
		node = leftRotate(node);
	}

	if(node->previous!=NULL){
        node->previous->height = 1 + max(height(node->previous->left),
                    height(node->previous->right));
        if (node->key < node->previous->key){
            node->previous->left = node;
            --node->previous->num_of_elements_on_left;
            node = repairNumOf(node->previous);
        }
        else if (node->key > node->previous->key){
            node->previous->right = node;
            --node->previous->num_of_elements_on_right;
            node = repairNumOf(node->previous);
        }
    }

    return node;
}

AVLtree::Node *AVLtree::rightRotate(Node *y)
{
	Node *x = y->left;
	Node *T2 = x->right;

	x->previous = y->previous;
	x->right = y;
	y->previous = x;
	y->left = T2;
	if (T2 != NULL)
		T2->previous = y;

	y->height = max(height(y->left), height(y->right)) + 1;
	y->num_of_elements_on_left = x->num_of_elements_on_right;
	x->height = max(height(x->left), height(x->right)) + 1;
	x->num_of_elements_on_right = y->num_of_elements_on_left + y->num_of_elements_on_right + 1;

	return x;
}

AVLtree::Node *AVLtree::leftRotate(Node *x)
{
	Node *y = x->right;
	Node *T2 = y->left;

	y->previous = x->previous;
	y->left = x;
	x->previous = y;
	x->right = T2;
	if (T2 != NULL)
		T2->previous = x;

	x->height = max(height(x->left), height(x->right)) + 1;
	x->num_of_elements_on_right = y->num_of_elements_on_left;
	y->height = max(height(y->left), height(y->right)) + 1;
	y->num_of_elements_on_left = x->num_of_elements_on_left + x->num_of_elements_on_right + 1;

	return y;
}

int AVLtree::getBalance(Node *N)
{
	if (N == NULL)
		return 0;
	return height(N->left) - height(N->right);
}


/// Funkcje przechodzace po drzewie
/// f kest funkcja ktora ma sie wykonac
/// na poszczegolnych wierzcholkach drzewa

void AVLtree::printF(){
    print Print;
    leftToRightOrder(Print);
}

void AVLtree::preOrder(const func &f){
    preOrder(root, f);
}

void AVLtree::preOrder(Node *node, const func &f)
{
	if (node != NULL)
	{
		f(node);
		preOrder(node->left, f);
		preOrder(node->right, f);
	}
}

void AVLtree::leftToRightOrder(const func &f){
    leftToRightOrder(root, f);
}

void AVLtree::addIndex(int i, int from, Node *node)
{
	if (node != NULL)
	{
        addIndex(i, from, node->right);
        if(node->key<from)return;
        node->key+=i;
        addIndex(i, from, node->left);
	}
}


void AVLtree::leftToRightOrder(Node *node, const func &f)
{
	if (node != NULL)
	{
		leftToRightOrder(node->left, f);
		f(node);
		leftToRightOrder(node->right, f);
	}
}

void AVLtree::rightToLeftOrder(Node *node, const func &f)
{
	if (node != NULL)
	{
		rightToLeftOrder(node->right, f);
		rightToLeftOrder(node->left, f);
		f(node);
	}
}


void AVLtree::fromTo(int from, int to, const func &f){
    id=0;
    fromTo(root, from, to, f);
}

void AVLtree::fromTo(Node *node, int from, int to, const func &f){
	if (node != NULL)
	{
		if (id > to) return;
		fromTo(node->left, from, to, f);
		if (id >= from){

			f(node);

		}
		++id;
		fromTo(node->right, from, to, f);
	}
}

void AVLtree::fromToKey(int from, int to, const func &f ){
    fromTo(root, from, to, f);
}

void AVLtree::fromToKey(Node *node, int from, int to, const func &f){
	if (node != NULL)
	{
		fromToKey(node->left, from, to, f);
		if (node->key > to) return;
		if (node->key >= from){

			f(node);

		}
		fromToKey(node->right, from, to, f);
	}
}

///
/// Konstruktor wewnetrznej klasy
///


AVLtree::Node::Node(){

}

AVLtree::Node::Node(int key, Node *previous){
	this->previous = previous;
	this->key = key;
	this->left = NULL;
	this->right = NULL;
	this->height = 1; 
	this->num_of_elements_on_left = 0;
	this->num_of_elements_on_right = 0;
}


AVLtree::Node::~Node(){

}
