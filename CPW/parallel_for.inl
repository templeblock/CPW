// ========================================================================== //
// The MIT License (MIT)                                                      //
//                                                                            //
// Copyright (c) 2016-2017 Jefferson Amstutz                                  //
//                                                                            //
// Permission is hereby granted, free of charge, to any person obtaining a    //
// copy of this software and associated documentation files (the "Software"), //
// to deal in the Software without restriction, including without limitation  //
// the rights to use, copy, modify, merge, publish, distribute, sublicense,   //
// and/or sell copies of the Software, and to permit persons to whom the      //
// Software is furnished to do so, subject to the following conditions:       //
//                                                                            //
// The above copyright notice and this permission notice shall be included in //
// in all copies or substantial portions of the Software.                     //
//                                                                            //
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR //
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   //
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    //
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER //
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    //
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        //
// DEALINGS IN THE SOFTWARE.                                                  //
// ========================================================================== //

#pragma once

#include <utility>

#ifdef CPW_TASKING_TBB
#  include <tbb/parallel_for.h>
#elif defined(CPW_TASKING_CILK)
#  include <cilk/cilk.h>
#elif defined(CPW_TASKING_INTERNAL)
#  include "ospcommon/tasking/TaskSys.h"
#endif

namespace CPW {

  template<typename TASK_T>
  inline void parallel_for_impl(int nTasks, TASK_T&& fcn)
  {
#ifdef CPW_TASKING_TBB
    tbb::parallel_for(0, nTasks, 1, fcn);
#elif defined(CPW_TASKING_CILK)
    cilk_for (int taskIndex = 0; taskIndex < nTasks; ++taskIndex) {
      fcn(taskIndex);
    }
#elif defined(CPW_TASKING_OMP)
# pragma omp parallel for schedule(dynamic)
    for (int taskIndex = 0; taskIndex < nTasks; ++taskIndex) {
      fcn(taskIndex);
    }
#elif defined(CPW_TASKING_INTERNAL)
    struct LocalTask : public Task {
      const TASK_T &t;
      LocalTask(const TASK_T& fcn) : Task("LocalTask"), t(fcn) {}
      void run(size_t taskIndex) override { t(taskIndex); }
    };

    Ref<LocalTask> task = new LocalTask(fcn);
    task->scheduleAndWait(nTasks);
#else // Debug (no tasking system)
    for (int taskIndex = 0; taskIndex < nTasks; ++taskIndex) {
      fcn(taskIndex);
    }
#endif
  }

} // namespace CPW
