namespace onart {

    struct BL_RBT_Node {
    private:
        template<class T>
        inline static void swap(T& a, T& b) {
            T c = b;
            b = a;
            a = c;
        }

        template<class T>
        inline static T* pointerXOR(T* a, T* b, T* c) {
            return reinterpret_cast<T*>((unsigned long long)(a) ^ (unsigned long long)(b) ^ (unsigned long long)(c));
        }

        template<class T>
        inline static T* pointerOR(T* a, T* b) {
            return (unsigned long long)(a) | (unsigned long long)(b);
        }

    public:
        enum class RB_ATTR { RED, BLACK } color;
        BL_RBT_Node* parent = nullptr;
        BL_RBT_Node* left = nullptr;
        BL_RBT_Node* right = nullptr;
        BL_RBT_Node* prev = nullptr;
        BL_RBT_Node* next = nullptr;

        inline BL_RBT_Node* root() {
            BL_RBT_Node* cursor = this;
            while (1) {
                if (!cursor->parent) return cursor;
                cursor = cursor->parent;
            }
        }

        // this node goes down
        inline void rotateLeft() {
            bool wasLeft = isLeft();
            BL_RBT_Node* gp = parent;
            parent = right;
            right->parent = gp;
            right = parent->left;
            if (right) right->parent = this;
            if (gp) {
                if (wasLeft) { gp->left = parent; }
                else { gp->right = parent; }
            }
            parent->left = this;
        }

        // this node goes down
        inline void rotateRight() {
            bool wasLeft = isLeft();
            BL_RBT_Node* gp = parent;
            parent = left;
            left->parent = gp;
            left = parent->right;
            if (left) left->parent = this;
            if (gp) {
                if (wasLeft) { gp->left = parent; }
                else { gp->right = parent; }
            }
            parent->right = this;
        }

        inline void insertLeft(BL_RBT_Node* newNode) {
            // if(left) return;
            left = newNode;
            newNode->color = RB_ATTR::RED;
            newNode->parent = this;
            newNode->prev = prev;
            if (newNode->prev) newNode->prev->next = newNode;
            newNode->next = this;
            prev = newNode;
            balanceOnInsert(newNode);
        }

        inline void insertRight(BL_RBT_Node* newNode) {
            // if(right) return;
            right = newNode;
            newNode->color = RB_ATTR::RED;
            newNode->parent = this;
            newNode->next = next;
            if (newNode->next) newNode->prev = newNode;
            next = newNode;
            newNode->prev = this;
            balanceOnInsert(newNode);
        }

        // Preserves all the rest of the iterators by changing links
        inline void detach() {
            if (left && right) { swapNode(prev); }

            if (left) { swapNode(prev); }
            else if (right) { swapNode(next); }

            if (prev) { prev->next = next; }
            if (next) { next->prev = prev; }

            if (!parent) return;

            bool deletedIsLeft = isLeft2();
            if (deletedIsLeft) parent->left = nullptr;
            else parent->right = nullptr;

            if (color == RB_ATTR::RED) return;

            // no children
            balanceOnErase(parent, deletedIsLeft);
        }

    private:
        inline bool isLeft() const { return parent && parent->left == this; }
        inline bool isLeft2() const { return parent->left == this; }
        inline BL_RBT_Node* grandParent() { return parent ? parent->parent : nullptr; }
        inline BL_RBT_Node* grandParent2() { return parent->parent; }
        inline BL_RBT_Node* sibling() { return parent ? pointerXOR(parent->left, parent->right, this) : nullptr; }
        inline BL_RBT_Node* sibling2() { return pointerXOR(parent->left, parent->right, this); }
        inline BL_RBT_Node* uncle() { return parent ? parent->sibling() : nullptr; }
        inline BL_RBT_Node* uncle2() { return parent->sibling(); }
        inline BL_RBT_Node* uncle3() { return parent->sibling2(); }
        inline bool isRed() { return this && this->color == RB_ATTR::RED; }
        inline bool isBlack() { return !isRed(); }
    private:
        inline static void balanceOnInsert(BL_RBT_Node* newNode) {
            while (newNode->parent) {
                if (newNode->parent->isBlack()) return;
                // newNode->parent->parent non-null
                BL_RBT_Node* newUncle = newNode->parent->sibling2();
                if (newUncle->isRed()) {
                    // newUncle non-null
                    newNode->parent->color = RB_ATTR::BLACK;
                    newUncle->color = RB_ATTR::BLACK;
                    newNode->parent->parent->color = RB_ATTR::RED;
                    newNode = newNode->parent->parent;
                    continue;
                }
                BL_RBT_Node* grand = newNode->parent->parent;
                newNode->parent->color = RB_ATTR::BLACK;
                newNode->parent->parent->color = RB_ATTR::RED;
                bool leftParent = newNode->parent->isLeft2();
                if (!newNode->isLeft2() && leftParent) { newNode->parent->rotateLeft(); }
                else if (newNode->isLeft2() && !leftParent) { newNode->parent->rotateRight(); }

                if (leftParent) { grand->rotateRight(); }
                else { grand->rotateLeft(); }
                return;
            }
            // newNode == root
            newNode->color = RB_ATTR::BLACK;
        }

        inline static void balanceOnErase(BL_RBT_Node* victim, bool lackLeft) {
            // meaning: victim's left or right height is lower
            while (victim) {
                BL_RBT_Node* s = lackLeft ? victim->right : victim->left;
                if (s && s->left->isRed() || s->right->isRed()) {
                    if (s->isLeft2()) {
                        if (s->left->isRed()) {
                            s->left->color = RB_ATTR::BLACK;
                            victim->rotateRight();
                            s->color = victim->color;
                            victim->color = RB_ATTR::BLACK;
                        }
                        else { // if(s->right->isRed())
                            s->color = victim->color;
                            s->right->color = RB_ATTR::BLACK;
                            s->rotateLeft();
                            victim->rotateRight();
                        }
                    }
                    else {
                        if (s->right->isRed()) {
                            s->right->color = RB_ATTR::BLACK;
                            victim->rotateLeft();
                            s->color = victim->color;
                            victim->color = RB_ATTR::BLACK;
                        }
                        else {
                            s->color = victim->color;
                            s->left->color = RB_ATTR::BLACK;
                            s->rotateRight();
                            victim->rotateLeft();
                        }
                    }
                    return;
                }
                else if (s->isBlack()) {
                    s->color = RB_ATTR::RED;
                    if (victim->color == RB_ATTR::RED) {
                        victim->color = RB_ATTR::BLACK;
                        return;
                    }
                    else {
                        victim = victim->parent;
                        lackLeft = victim->isLeft2();
                        continue;
                    }
                }
                else { // s is red node (victim (== s->parent) is black node)
                    s->color = RB_ATTR::BLACK;
                    victim->color = RB_ATTR::RED;
                    if (s->isLeft2()) { victim->rotateRight(); }
                    else { victim->rotateLeft(); }
                    // leave victim & lackLeft unchanged
                    continue;
                }
            }
        }
        
        // this is valid only when 'that' is in-order predecessor or in-order successor && that is descendant of this
        inline void swapNode(BL_RBT_Node* that) {
            bool thisWasLeft = isLeft();
            bool thatWasLeft = that->isLeft();

            swap(parent, that->parent);
            swap(left, that->left);
            swap(right, that->right);
            swap(color, that->color);

            if (parent) {
                if (parent == this) { parent = that; }
                if (thatWasLeft) parent->left = this;
                else parent->right = this;
            }

            if (left) left->parent = this;
            if (right) right->parent = this;

            if (that->parent) {
                if (thisWasLeft) that->parent->left = that;
                else that->parent->right = that;
            }

            if (that->left) that->left->parent = that;
            if (that->right) that->right->parent = that;

            // assert(that == prev || that == next)
            BL_RBT_Node* serial[4]{};
            if (that->prev == this) {
                serial[1] = this;
                serial[2] = that;
            }
            else { // that->next == this
                serial[1] = that;
                serial[2] = this;
            }
            serial[0] = serial[1]->prev;
            serial[3] = serial[2]->next;

            serial[0]->next = serial[2];
            serial[2]->next = serial[1];
            serial[1]->next = serial[3];
            serial[3]->prev = serial[1];
            serial[1]->prev = serial[2];
            serial[2]->prev = serial[0];
        }
    };

    template<class K, class V>
    struct kvpair {
        K first;
        V second;
    };

    template<class K, class V>
    struct ckvpair {
        const K first;
        V second;
        inline ckvpair(const K& first, const V& second) :first(first), second(second) {}
        inline ckvpair(const ckvpair& rhs) : first(rhs.first), second(rhs.second) {}
        inline ckvpair(const kvpair<K, V>& rhs) : first(rhs.first), second(rhs.second) {}
    };

    template<class T>
    class set {
    public:
        inline unsigned long long size() const { return count; }
        inline set(size_t reserve = 0) :root(nullptr), head(nullptr), capacity(reserve), poolUsage(0), count(0) {
            if (reserve) {
                reserved = reinterpret_cast<ContainerNode*>(std::malloc((sizeof(ContainerNode) + sizeof(ContainerNode*)) * reserve));
                istack = reinterpret_cast<ContainerNode**>(reserved + reserve);
                for (size_t i = 0; i < reserve; i++) {
                    istack[i] = reserved + i;
                }
            }
            head = &endNode;
            endNode.next = &endNode;
            endNode.prev = &endNode;
        }
        inline ~set() {
            ContainerNode* cursor = head;
            while (cursor && cursor != &endNode) {
                cursor->value.~T();

                ContainerNode* next = static_cast<ContainerNode*>(cursor->next);
                deallocate(cursor);
                cursor = next;
            }
            std::free(reserved);
        }

        inline set(set&& rhs) :__rb_container(reserve), capacity(rhs.capacity) {
            root = rhs.root;    rhs.root = nullptr;
            head = rhs.head;    rhs.head = nullptr;
            endNode.prev = rhs.endNode.prev;
            endNode.next = &endNode;
            rhs.endNode.prev = rhs.endNode.next = nullptr;
            if (endNode.prev) endNode.prev->next = &endNode;
            reserved = rhs.reserved;    rhs.reserved = nullptr;
            istack = rhs.istack;    rhs.istack = nullptr;
            poolUsage = rhs.poolUsage;  rhs.poolUsage = 0;
            count = rhs.count; rhs.count = 0;
        }

        inline set(const set&) = delete;
        inline set& operator=(const set&) = delete;
    private:
        struct ContainerNode : public BL_RBT_Node { T value; };

        ContainerNode* root = nullptr;
        ContainerNode* head = nullptr;
        ContainerNode endNode;
        ContainerNode* reserved = nullptr;
        ContainerNode** istack = nullptr;
        const unsigned long long capacity;
        unsigned long long poolUsage;
        unsigned long long count;

        inline ContainerNode* allocate() {
            count++;
            if (capacity > poolUsage) { return istack[poolUsage++]; }
            else { return reinterpret_cast<ContainerNode*>(std::malloc(sizeof(ContainerNode))); }
        }
        inline void deallocate(ContainerNode* p) {
            count--;
            if (p - reserved < 0 || p - reserved > capacity) { std::free(p); }
            else { istack[--poolUsage] = p; }
        }
    public:
        class iterator {
            friend class set;
        public:
            inline iterator() :_container(nullptr), node(nullptr) {}
            inline iterator(const iterator& other) : _container(other._container), node(other.node) {}
            inline bool operator==(const iterator& other) const { return node == other.node; }
            inline bool operator!=(const iterator& other) const { return !operator==(other); }
            inline iterator& operator++() { node = static_cast<ContainerNode*>(node->next); return *this; }
            inline iterator operator++(int) { ContainerNode* prv = node; node = static_cast<ContainerNode*>(node->next); return iterator(_container, prv); }
            inline iterator& operator--() { node = node->prev; return *this; }
            inline iterator operator--(int) { ContainerNode* prv = node; node = static_cast<ContainerNode*>(node->prev); return iterator(_container, prv); }
            inline T& operator*() { return node->value; }
            inline const T& operator*() const { return node->value; }
            inline T* operator->() { return &(node->value); }
            inline const T* operator->() const { return &(node->value); }
        private:
            set* _container;
            ContainerNode* node;
            inline iterator(set* c, ContainerNode* n) :_container(c), node(n) {}
        };
        inline iterator begin() { return iterator(this, head); }
        inline iterator end() { return iterator(this, &endNode); }

        inline iterator find(const T& key) {
            ContainerNode* cursor = root;
            while (cursor) {
                if (cursor->value < key) {
                    cursor = static_cast<ContainerNode*>(cursor->right);
                }
                else if (key < cursor->value) {
                    cursor = static_cast<ContainerNode*>(cursor->left);
                }
                else {
                    return iterator(this, cursor);
                }
            }
            return end();
        }

        template<class U>
        inline iterator lowerBound(const U& key) {
            ContainerNode* cursor = root;
            ContainerNode* candid = &endNode;
            while (cursor) {
                if (cursor->value < key) {
                    cursor = cursor->right;
                }
                else if (key < cursor->value) {
                    candid = cursor;
                    cursor = cursor->left;
                }
                else {
                    return iterator(this, cursor);
                }
            }
            return iterator(this, candid);
        }

        template<class U>
        inline iterator upperBound(const U& key) {
            ContainerNode* cursor = root;
            ContainerNode* candid = &endNode;
            while (cursor) {
                if (cursor->value < key) {
                    cursor = cursor->right;
                }
                else if (key < cursor->value) {
                    candid = cursor;
                    cursor = cursor->left;
                }
                else {
                    return iterator(this, cursor->next);
                }
            }
            return iterator(this, candid);
        }

        inline iterator insert(const T& key) {
            if (!root) {
                ContainerNode* newNode = allocate();
                newNode->left = newNode->right = newNode->parent = nullptr;
                new(&newNode->value) T(key);
                root = newNode;
                head = root;
                head->prev = &endNode;
                head->next = &endNode;
                endNode.prev = head;
                return iterator(this, newNode);
            }
            else {
                ContainerNode* cursor = root;
                ContainerNode* insertionPoint = cursor;
                while (cursor) {
                    insertionPoint = cursor;
                    if (cursor->value < key) { cursor = static_cast<ContainerNode*>(cursor->right); }
                    else if (key < cursor->value) { cursor = static_cast<ContainerNode*>(cursor->left); }
                    else { return iterator(this, cursor); }
                }
                ContainerNode* newNode = allocate();
                newNode->left = newNode->right = newNode->parent = nullptr;
                if (insertionPoint->value < key) { insertionPoint->insertRight(newNode); }
                else { 
                    if (insertionPoint == head) { head = newNode; }
                    insertionPoint->insertLeft(newNode);
                }
                new (&newNode->value) T(key);
                root = static_cast<ContainerNode*>(root->root());
                return iterator(this, newNode);
            }
        }

        inline iterator erase(iterator pos) {
            if (pos == end() || pos._container != this) return end();
            iterator toErase = pos++;
            ContainerNode* nextRootFinder = root;
            if (toErase.node == root) {
                nextRootFinder = static_cast<ContainerNode*>(root->left ? root->left : root->right);
            }
            toErase.node->detach();
            toErase.node->value.~T();
            deallocate(toErase.node);
            if (count == 0) {
                root = nullptr;
                head = &endNode;
            }
            else if (nextRootFinder) {
                root = static_cast<ContainerNode*>(nextRootFinder->root());
            }
            return pos;
        }

        inline size_t erase(const T& key) {
            iterator it = find(key);
            if (it == end()) { return 0; }
            erase(it);
            return 1;
        }
    };

    template<class K, class V>
    class map {
        using T = ckvpair<K, V>;
        using mutableT = kvpair<K, V>;
    public:
        inline map(size_t reserve = 0) :root(nullptr), head(nullptr), capacity(reserve), poolUsage(0), count(0) {
            if (reserve) {
                reserved = reinterpret_cast<ContainerNode*>(std::malloc((sizeof(ContainerNode) + sizeof(ContainerNode*)) * reserve));
                istack = reinterpret_cast<ContainerNode**>(reserved + reserve);
                for (size_t i = 0; i < reserve; i++) {
                    istack[i] = reserved + i;
                }
            }
            head = &endNode;
            endNode.next = &endNode;
            endNode.prev = &endNode;
        }
        inline ~map() {
            ContainerNode* cursor = head;
            while (cursor && cursor != &endNode) {
                cursor->value.~T();

                ContainerNode* next = static_cast<ContainerNode*>(cursor->next);
                deallocate(cursor);
                cursor = next;
            }
        }

        inline map(map&& rhs) :__rb_container(reserve), capacity(rhs.capacity) {
            root = rhs.root;    rhs.root = nullptr;
            head = rhs.head;    rhs.head = nullptr;
            endNode.prev = rhs.endNode.prev;
            endNode.next = &endNode;
            rhs.endNode.prev = rhs.endNode.next = nullptr;
            if (endNode.prev) endNode.prev->next = &endNode;
            reserved = rhs.reserved;    rhs.reserved = nullptr;
            istack = rhs.istack;    rhs.istack = nullptr;
            poolUsage = rhs.poolUsage;  rhs.poolUsage = 0;
            count = rhs.count; rhs.count = 0;
        }

        inline map(const map&) = delete;
        inline map& operator=(const map&) = delete;

    private:
        struct ContainerNode : public BL_RBT_Node { 
            T value;
            ContainerNode():value(K(),V()) {}
        };

        ContainerNode* root = nullptr;
        ContainerNode* head = nullptr;
        ContainerNode endNode;
        ContainerNode* reserved = nullptr;
        ContainerNode** istack = nullptr;
        const unsigned long long capacity;
        unsigned long long poolUsage;
        unsigned long long count;

        inline ContainerNode* allocate() {
            count++;
            if (capacity > poolUsage) { return istack[poolUsage++]; }
            else { return reinterpret_cast<ContainerNode*>(std::malloc(sizeof(ContainerNode))); }
        }
        inline void deallocate(ContainerNode* p) {
            count--;
            if (p - reserved < 0 || p - reserved > capacity) { std::free(p); }
            else { istack[--poolUsage] = p; }
        }
        inline ContainerNode* insertEmpty(const K& key) {
            if (!root) {
                ContainerNode* newNode = allocate();
                newNode->left = newNode->right = newNode->parent = nullptr;
                root = newNode;
                head = root;
                head->prev = &endNode;
                head->next = &endNode;
                endNode.prev = head;
                return newNode;
            }
            else {
                ContainerNode* cursor = root;
                ContainerNode* insertionPoint = cursor;
                while (cursor) {
                    insertionPoint = cursor;
                    if (cursor->value.first < key) { cursor = static_cast<ContainerNode*>(cursor->right); }
                    else if (key < cursor->value.first) { cursor = static_cast<ContainerNode*>(cursor->left); }
                    else { return cursor; }
                }
                ContainerNode* newNode = allocate();
                newNode->left = newNode->right = newNode->parent = nullptr;
                if (insertionPoint->value.first < key) { insertionPoint->insertRight(newNode); }
                else { insertionPoint->insertLeft(newNode); }
                root = static_cast<ContainerNode*>(root->root());
                return newNode;
            }
        }
    public:
        class iterator {
            friend class map;
        public:
            inline iterator() :_container(nullptr), node(nullptr) {}
            inline iterator(const iterator& other) : _container(other._container), node(other.node) {}
            inline bool operator==(const iterator& other) const { return node == other.node; }
            inline bool operator!=(const iterator& other) const { return !operator==(other); }
            inline iterator& operator++() { node = static_cast<ContainerNode*>(node->next); return *this; }
            inline iterator operator++(int) { ContainerNode* prv = node; node = static_cast<ContainerNode*>(node->next); return iterator(_container, prv); }
            inline iterator& operator--() { node = node->prev; return *this; }
            inline iterator operator--(int) { ContainerNode* prv = node; node = static_cast<ContainerNode*>(node->prev); return iterator(_container, prv); }
            inline T& operator*() { return node->value; }
            inline const T& operator*() const { return node->value; }
            inline T* operator->() { return &(node->value); }
            inline const T* operator->() const { return &(node->value); }
        private:
            inline iterator(map* c, ContainerNode* n) :_container(c), node(n) {}
            map* _container;
            ContainerNode* node;
        };

        inline iterator begin() { return iterator(this, head); }
        inline iterator end() { return iterator(this, &endNode); }

        inline iterator find(const K& key) {
            ContainerNode* cursor = root;
            while (cursor) {
                if (cursor->value.first < key) {
                    cursor = static_cast<ContainerNode*>(cursor->right);
                }
                else if (key < cursor->value.first) {
                    cursor = static_cast<ContainerNode*>(cursor->left);
                }
                else {
                    return iterator(this, cursor);
                }
            }
            return end();
        }

        template<class U>
        inline iterator lowerBound(const U& key) {
            ContainerNode* cursor = root;
            ContainerNode* candid = &endNode;
            while (cursor) {
                if (cursor->value.first < key) {
                    cursor = cursor->right;
                }
                else if (key < cursor->value.first) {
                    candid = cursor;
                    cursor = cursor->left;
                }
                else {
                    return iterator(this, cursor);
                }
            }
            return iterator(this, candid);
        }

        template<class U>
        inline iterator upperBound(const U& key) {
            ContainerNode* cursor = root;
            ContainerNode* candid = &endNode;
            while (cursor) {
                if (cursor->value.first < key) {
                    cursor = cursor->right;
                }
                else if (key < cursor->value.first) {
                    candid = cursor;
                    cursor = cursor->left;
                }
                else {
                    return iterator(this, cursor->next);
                }
            }
            return iterator(this, candid);
        }

        inline iterator insert(const T& newV) {
            ContainerNode* newNode = insertEmpty(newV.first);
            new (&newNode->value) T(newV);
            return iterator(this, newNode);
        }

        inline iterator insert(const mutableT& newV) {
            ContainerNode* newNode = insertEmpty(newV.first);
            new (&newNode->value) T(newV);
            return iterator(this, newNode);
        }

        inline iterator erase(iterator pos) {
            if (pos == end() || pos._container != this) return end();
            iterator toErase = pos++;
            ContainerNode* nextRootFinder = root;
            if (toErase.node == root) {
                nextRootFinder = static_cast<ContainerNode*>(root->left ? root->left : root->right);
            }
            toErase.node->detach();
            toErase.node->value.~T();
            deallocate(toErase.node);
            if (count == 0) {
                root = nullptr;
                head = &endNode;
            }
            else if (nextRootFinder) {
                root = static_cast<ContainerNode*>(nextRootFinder->root());
            }
            return pos;
        }

        inline size_t erase(const K& key) {
            iterator it = find(key);
            if (it == end()) { return 0; }
            erase(it);
            return 1;
        }

        inline V& operator[](const K& key) {
            size_t beforeCount = count;
            ContainerNode* newNode = insertEmpty(key);
            if (count > beforeCount) {
                new(newNode->value) T(key, {});
            }
            return newNode->value.second;
        }
    };
}