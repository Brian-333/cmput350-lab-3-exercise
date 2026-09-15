#include <cstdlib>
#include "SharedPtr.h"
#include <iostream>
#include <cassert>

int main() {
   // Check for new shared pointer
   SharedPtr<int> ptr1(new int(10));
   assert(ptr1.useCount() == 1);
   assert(*ptr1 == 10);

   // Check for assignment
   SharedPtr<int> ptr2(new int(20));
   assert(ptr2.useCount() == 1);
   ptr1 = ptr2;
   assert(ptr1.useCount() == 2);
   assert(*ptr1 == 20);
   assert(ptr2.get() != nullptr);
   
   // Check for makeShared
   SharedPtr<int> ptr3 = makeShared<int>(30);
   assert(ptr3.useCount() == 1);
   assert(*ptr3 == 30);

   // Check for copy constructor
   SharedPtr<int> ptr4(ptr1);
   assert(ptr4.useCount() == 3);
   assert(*ptr4 == 20);

   // check for move constructor
   SharedPtr<int> ptr5(std::move(ptr1));
   assert(ptr1.useCount() == 0);
   assert(ptr1.get() == nullptr);
   assert(ptr5.useCount() == 3);
   assert(*ptr5 == 20);

   // check for move assignment
   ptr1 = std::move(ptr5);
   assert(ptr1.useCount() == 3);
   assert(*ptr1 == 20);
   assert(ptr5.useCount() == 0);
   assert(ptr5.get() == nullptr);

   // Check for reset
   ptr1.reset();
   assert(ptr1.useCount() == 0);
   assert(ptr1.get() == nullptr);

   // Check for dealing with both embedded and non-embedded types
   SharedPtr<int> ptr6 = makeSharedBasic<int>(40);
   SharedPtr<int> ptr7 = makeShared<int>(50);
   ptr7 = ptr6;
   assert(ptr6.useCount() == 2);
   assert(ptr7.useCount() == 2);
   assert(*ptr6 == 40);
   assert(*ptr7 == 40);

   std::cout << "All tests passed\n";
   return EXIT_SUCCESS;
}
