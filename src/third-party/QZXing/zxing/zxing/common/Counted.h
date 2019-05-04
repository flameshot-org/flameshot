// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
#ifndef ZXING_COUNTED_H
#define ZXING_COUNTED_H

/*
 *  Copyright 2010 ZXing authors All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>

namespace zxing {

/* base class for reference-counted objects */
class Counted {
private:
  unsigned int count_;
public:
  Counted() :
      count_(0) {
  }
  virtual ~Counted() {
  }
  Counted *retain() {
    count_++;
    return this;
  }
  void release() {
    count_--;
    if (count_ == 0) {
      count_ = 0xDEADF001;
      delete this;
    }
  }


  /* return the current count for denugging purposes or similar */
  int count() const {
    return count_;
  }
};

/* counting reference to reference-counted objects */
template<typename T> class Ref {
private:
public:
  T *object_;
  explicit Ref(T *o = 0) :
      object_(0) {
    reset(o);
  }
  Ref(const Ref &other) :
      object_(0) {
    reset(other.object_);
  }

  template<class Y>
  Ref(const Ref<Y> &other) :
      object_(0) {
    reset(other.object_);
  }

  ~Ref() {
    if (object_) {
      object_->release();
    }
  }

  void reset(T *o) {
    if (o) {
      o->retain();
    }
    if (object_ != 0) {
      object_->release();
    }
    object_ = o;
  }
  Ref& operator=(const Ref &other) {
    reset(other.object_);
    return *this;
  }
  template<class Y>
  Ref& operator=(const Ref<Y> &other) {
    reset(other.object_);
    return *this;
  }
  Ref& operator=(T* o) {
    reset(o);
    return *this;
  }
  template<class Y>
  Ref& operator=(Y* o) {
    reset(o);
    return *this;
  }

  T& operator*() {
    return *object_;
  }
  T* operator->() const {
    return object_;
  }
  operator T*() const {
    return object_;
  }

  bool operator==(const T* that) {
    return object_ == that;
  }
  bool operator==(const Ref &other) const {
    return object_ == other.object_ || *object_ == *(other.object_);
  }
  template<class Y>
  bool operator==(const Ref<Y> &other) const {
    return object_ == other.object_ || *object_ == *(other.object_);
  }

  bool operator!=(const T* that) {
    return !(*this == that);
  }

  bool empty() const {
    return object_ == 0;
  }
};

}

#endif // ZXING_COUNTED_H
