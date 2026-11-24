static void ReplaceServiceTransportChildPilot(ServiceTransportTreeNodeObservedPilot* head,
    ServiceTransportTreeNodeObservedPilot* node, ServiceTransportTreeNodeObservedPilot* replacement)
{
    replacement->parent = node->parent;
    if (node->parent == head)
        head->parent = replacement;
    else if (node == node->parent->left)
        node->parent->left = replacement;
    else
        node->parent->right = replacement;
}

// 20110867.
static void RotateServiceTransportLeftPilot(ServiceTransportTreeNodeObservedPilot* head,
    ServiceTransportTreeNodeObservedPilot* node)
{
    ServiceTransportTreeNodeObservedPilot* pivot = node->right;
    node->right = pivot->left;
    if (node->right != head) node->right->parent = node;
    ReplaceServiceTransportChildPilot(head, node, pivot);
    pivot->left = node;
    node->parent = pivot;
}

// 201108ba.
static void RotateServiceTransportRightPilot(ServiceTransportTreeNodeObservedPilot* head,
    ServiceTransportTreeNodeObservedPilot* node)
{
    ServiceTransportTreeNodeObservedPilot* pivot = node->left;
    node->left = pivot->right;
    if (node->left != head) node->left->parent = node;
    ReplaceServiceTransportChildPilot(head, node, pivot);
    pivot->right = node;
    node->parent = pivot;
}

// 20116c87. Zero is red and one is black, including the sentinel.
static void BalanceServiceTransportInsertPilot(ServiceTransportTreeNodeObservedPilot* head,
    ServiceTransportTreeNodeObservedPilot* node)
{
    while (node->parent->color == 0) {
        ServiceTransportTreeNodeObservedPilot* parent = node->parent;
        ServiceTransportTreeNodeObservedPilot* grandparent = parent->parent;
        bool parentIsLeft = parent == grandparent->left;
        ServiceTransportTreeNodeObservedPilot* uncle = parentIsLeft ? grandparent->right : grandparent->left;
        if (uncle->color == 0) {
            parent->color = uncle->color = 1;
            grandparent->color = 0;
            node = grandparent;
            continue;
        }
        if (parentIsLeft && node == parent->right) {
            RotateServiceTransportLeftPilot(head, parent);
            parent = node;
        } else if (!parentIsLeft && node == parent->left) {
            RotateServiceTransportRightPilot(head, parent);
            parent = node;
        }
        parent->color = 1;
        grandparent->color = 0;
        if (parentIsLeft)
            RotateServiceTransportRightPilot(head, grandparent);
        else
            RotateServiceTransportLeftPilot(head, grandparent);
        break;
    }
    head->parent->color = 1;
}

// 201165f2. A single signed-key walk replaces the lower-bound and hint adapters.
static BYTE** FindOrInsertServiceTransportClientPilot(LONG key)
{
    ServiceTransportTreeObservedPilot* tree = g_serviceTransportTreePilot;
    if (tree == NULL) return NULL;
    ServiceTransportTreeNodeObservedPilot* head = tree->head;
    ServiceTransportTreeNodeObservedPilot* parent = head;
    bool insertLeft = true;
    for (ServiceTransportTreeNodeObservedPilot* node = head->parent; node != head;) {
        if (key == node->key) return &node->client;
        parent = node;
        insertLeft = key < node->key;
        node = insertLeft ? node->left : node->right;
    }

    if (tree->count > 0x1ffffffdu) return NULL;
    ServiceTransportTreeNodeObservedPilot* inserted = static_cast<ServiceTransportTreeNodeObservedPilot*>(
        AllocateServiceClientMemoryPilot(0, 0, sizeof(ServiceTransportTreeNodeObservedPilot)));
    if (inserted == NULL) return NULL;
    inserted->left = inserted->right = head;
    inserted->parent = parent;
    inserted->key = key;
    inserted->client = NULL;
    inserted->color = inserted->sentinel = 0;
    ++tree->count;
    if (parent == head) {
        head->parent = head->left = head->right = inserted;
    } else if (insertLeft) {
        parent->left = inserted;
        if (parent == head->left) head->left = inserted;
    } else {
        parent->right = inserted;
        if (parent == head->right) head->right = inserted;
    }
    BalanceServiceTransportInsertPilot(head, inserted);
    return &inserted->client;
}
