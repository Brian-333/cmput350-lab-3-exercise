#ifndef SHARED_PTR_HEADER
#define SHARED_PTR_HEADER

#include <utility>
#include <cassert>

class ControlBlockBase {
public:
    ControlBlockBase() : currentRefCount(0) {};

    // dtor is virtual, so that we can call derived class's dtor from a ptr to this base class.
    virtual ~ControlBlockBase() {};

    // pure virtual function; must be overriden by derived classes
    virtual void* managedAddress() = 0;

    // Delete copies, which also implicitly deletes moves.
    ControlBlockBase(const ControlBlockBase&) = delete;
    ControlBlockBase& operator=(const ControlBlockBase&) = delete;

    long increment()
    {
        return ++currentRefCount;
    }

    long decrement()
    {
        return --currentRefCount;
    }

    long refCount() const
    {
        return currentRefCount;
    }

private:
    long currentRefCount;
};


template <typename T>
class ControlBlock : public ControlBlockBase {
public:
    explicit ControlBlock(T* inputManagedAddress = nullptr) : currentManagedAddress(inputManagedAddress) {};
    ~ControlBlock() override { delete currentManagedAddress; };
    void* managedAddress() override { return currentManagedAddress; };
private:
    T* currentManagedAddress;
};

template <typename T>
class ControlBlockEmbedded : public ControlBlockBase {
public:
    template <typename... Args>
    explicit ControlBlockEmbedded(Args&&... args) : currentValue(std::forward<Args>(args)...) {};
    ~ControlBlockEmbedded() override = default;
    void* managedAddress() override { return &currentValue; };
private:
    T currentValue;
};

template <typename T>
class SharedPtr {
public:
    // Default constructor.
    explicit SharedPtr() : controlBlock(nullptr), managedAddress(nullptr) {};
    // Constructor for a new control block
    explicit SharedPtr(T* managedAddress) : controlBlock(new ControlBlock<T>(managedAddress)), managedAddress(managedAddress) {
        controlBlock->increment();
    };
    template <typename U, typename... Args> friend SharedPtr<U> makeShared(Args&&... args);
    
    // Destructor.
    ~SharedPtr() {
        reset();
    };
    // Copy constructor.
    SharedPtr(const SharedPtr<T>& other) : controlBlock(other.controlBlock), managedAddress(other.managedAddress) {
        controlBlock->increment();
    };
    // Copy assignment operator.
    SharedPtr<T>& operator=(const SharedPtr<T>& other) {
        if (this == &other) {
            return *this;
        }
        // Decrement the ref count of the current control block.
        reset();
        // Assign the new control block and managed address.
        controlBlock = other.controlBlock;
        managedAddress = other.managedAddress;
        controlBlock->increment();
        return *this;
    };
    // Move constructor.
    SharedPtr(SharedPtr<T>&& other) : controlBlock(other.controlBlock), managedAddress(other.managedAddress) {
        other.controlBlock = nullptr;
        other.managedAddress = nullptr;
    };
    // Move assignment operator.
    SharedPtr<T>& operator=(SharedPtr<T>&& other) {
        if (this == &other) {
            return *this;
        }
        // Decrement the ref count of the current control block.
        reset();
        // Assign the new control block and managed address.
        controlBlock = other.controlBlock;
        managedAddress = other.managedAddress;
        other.controlBlock = nullptr;
        other.managedAddress = nullptr;
        return *this;
    };

    // Dereference operator.
    T& operator*() const {
        assert(managedAddress != nullptr);
        return *managedAddress;
    };

    T* operator->() const {
        assert(managedAddress != nullptr);
        return managedAddress;
    };

    // Get the managed address.
    T* get() const {
        return managedAddress;
    };

    bool operator==(const SharedPtr<T>& other) const {
        return managedAddress == other.managedAddress;
    };

    // Conversion to bool.
    explicit operator bool() const { return managedAddress != nullptr; };

    void swap(SharedPtr<T>& other) {
        if (this == &other) {
            return;
        }
        std::swap(controlBlock, other.controlBlock);
        std::swap(managedAddress, other.managedAddress);
    };

    void reset() {
        if (controlBlock) {
            controlBlock->decrement();
            if (controlBlock->refCount() == 0) {
                delete controlBlock;
            }
        }
        controlBlock = nullptr;
        managedAddress = nullptr;
    };

    void reset(T* other) {
        if (other == managedAddress) {
            return;
        }
        // Reset the current control block and managed address.
        reset();
        // Assign the new control block and managed address.
        controlBlock = new ControlBlock<T>(other);
        managedAddress = other;
        controlBlock->increment();
    };

    long useCount() const {
        return controlBlock ? controlBlock->refCount() : 0;
    };

private:
    ControlBlockBase* controlBlock;
    T* managedAddress{};
    // Constructor from managed value with existing control block.
    SharedPtr(T* mangedAddress, ControlBlockBase* controlblock) : controlBlock(controlblock), managedAddress(mangedAddress) {
        controlBlock->increment();
    };
};

template <typename T, typename... Args> SharedPtr<T> makeSharedBasic(Args&&... args) {
    return SharedPtr<T>(new T(std::forward<Args>(args)...));
};

template <typename T, typename... Args> SharedPtr<T> makeShared(Args&&... args) {
    // Create a new control block for the embedded type.
    ControlBlockBase* controlBlock = new ControlBlockEmbedded<T>(std::forward<Args>(args)...);
    // pass the managed address created inside the control block.
    return SharedPtr<T>(static_cast<T*>(controlBlock->managedAddress()), controlBlock);
}

#endif
