#pragma once

#include <pthread.h>

class Mutex {
 public:
    Mutex() = default;
    ~Mutex() {
        pthread_mutex_destroy(&mutex_);
    }
    bool lock() {
        return pthread_mutex_lock(&mutex_) == 0;
    }
    bool unlock() {
        return pthread_mutex_unlock(&mutex_) == 0;
    }

 private:
    // 这种用macro初始化的写法, 区别就是没有错误检测
    pthread_mutex_t mutex_ = PTHREAD_MUTEX_INITIALIZER;
};

class RwLock {
 public:
    RwLock() = default;
    ~RwLock() {
        pthread_rwlock_destroy(&rwlock_);
    }
    bool rdlock() {
        return pthread_rwlock_rdlock(&rwlock_) == 0;
    }
    bool wrlock() {
        return pthread_rwlock_wrlock(&rwlock_) == 0;
    }
    bool unlock() {
        return pthread_rwlock_unlock(&rwlock_) == 0;
    }

 private:
    pthread_rwlock_t rwlock_ = PTHREAD_RWLOCK_INITIALIZER;
};

class SpinLock {
 public:
    SpinLock() {
        int ret = pthread_spin_init(&spinlock_, 0);
        if (ret != 0) {
            // error handle
        }
    }
    ~SpinLock() {
        pthread_spin_destroy(&spinlock_);
    }
    bool lock() {
        return pthread_spin_lock(&spinlock_) == 0;
    }
    bool unlock() {
        return pthread_spin_unlock(&spinlock_) == 0;
    }

 private:
    pthread_spinlock_t spinlock_;
};
