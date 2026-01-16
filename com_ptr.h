#pragma once
#include <type_traits>

template <typename T>
class SmartPtr {
    static_assert(std::is_base_of<IUnknown, T>::value, "T must inherit from IUnknown");
public:
    SmartPtr() noexcept : ptr_(nullptr) {}
    SmartPtr(T* ptr) noexcept : ptr_(ptr) {
        if (ptr_) ptr_->AddRef();
    }
    SmartPtr(const SmartPtr& other) noexcept : ptr_(other.ptr_) {
        if (ptr_) ptr_->AddRef();
    }
    SmartPtr(SmartPtr&& other) noexcept : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }
    ~SmartPtr() {
        if (ptr_) ptr_->Release();
    }
    SmartPtr& operator=(const SmartPtr& other) noexcept {
        if (this != &other) {
            if (ptr_) ptr_->Release();
            ptr_ = other.ptr_;
            if (ptr_) ptr_->AddRef();
        }
        return *this;
    }
    SmartPtr& operator=(SmartPtr&& other) noexcept {
        if (this != &other) {
            if (ptr_) ptr_->Release();
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }
    T* operator->() const noexcept { return ptr_; }
    T& operator*() const noexcept { return *ptr_; }
    T* get() const noexcept { return ptr_; }
    void reset(T* ptr = nullptr) noexcept {
        if (ptr_) ptr_->Release();
        ptr_ = ptr;
        if (ptr_) ptr_->AddRef();
    }
    T** operator&() noexcept { return &ptr_; }
    explicit operator bool() const noexcept { return ptr_ != nullptr; }
private:
    T* ptr_;
};
