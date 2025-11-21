#include <iostream>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <functional>
#include <memory>
#include <unistd.h>

using TaskFunc = std::function<void()>;
using ReleaseFunc = std::function<void()>;
class TimerTask{
    private:
        uint64_t _id;       // 定时器任务对象ID
        uint32_t _timeout;  //定时任务的超时时间
        bool _canceled;     // false-表示没有被取消， true-表示被取消
        TaskFunc _task_cb;  //定时器对象要执行的定时任务
        ReleaseFunc _release; //用于删除TimerWheel中保存的定时器对象信息
    public:
        TimerTask(uint64_t id, uint32_t delay, const TaskFunc &cb): 
            _id(id), _timeout(delay), _task_cb(cb), _canceled(false) {}
        ~TimerTask() { 
            if (_canceled == false) _task_cb(); 
            _release(); 
        }
        void Cancel() { _canceled = true; }
        void SetRelease(const ReleaseFunc &cb) { _release = cb; }
        uint32_t DelayTime() { return _timeout; }
};

class TimerWheel {
    private:
        using WeakTask = std::weak_ptr<TimerTask>;
        using PtrTask = std::shared_ptr<TimerTask>;
        int _tick;      //当前的秒针，走到哪里释放哪里，释放哪里，就相当于执行哪里的任务
        int _capacity;  //表盘最大数量---其实就是最大延迟时间
        std::vector<std::vector<PtrTask>> _wheel;
        std::unordered_map<uint64_t, WeakTask> _timers;
    private:
        void RemoveTimer(uint64_t id) {
            auto it = _timers.find(id);
            if (it != _timers.end()) {
                _timers.erase(it);
            }
        }
    public:
        TimerWheel():_capacity(60), _tick(0), _wheel(_capacity) {}
        void TimerAdd(uint64_t id, uint32_t delay, const TaskFunc &cb) {
            PtrTask pt(new TimerTask(id, delay, cb));
            pt->SetRelease(std::bind(&TimerWheel::RemoveTimer, this, id));
            int pos = (_tick + delay) % _capacity;
            _wheel[pos].push_back(pt);
            _timers[id] = WeakTask(pt);
        }
        //刷新/延迟定时任务
        void TimerRefresh(uint64_t id) {
            //通过保存的定时器对象的weak_ptr构造一个shared_ptr出来，添加到轮子中
            auto it = _timers.find(id);
            if (it == _timers.end()) {
                return;//没找着定时任务，没法刷新，没法延迟
            }
            PtrTask pt = it->second.lock();//lock获取weak_ptr管理的对象对应的shared_ptr
            int delay = pt->DelayTime();
            int pos = (_tick + delay) % _capacity;
            _wheel[pos].push_back(pt);
        }
        void TimerCancel(uint64_t id) {
            auto it = _timers.find(id);
            if (it == _timers.end()) {
                return;//没找着定时任务，没法刷新，没法延迟
            }
            PtrTask pt = it->second.lock();
            if (pt) pt->Cancel();
        }
        //这个函数应该每秒钟被执行一次，相当于秒针向后走了一步
        void RunTimerTask() {
            _tick = (_tick + 1) % _capacity;
            _wheel[_tick].clear();//清空指定位置的数组，就会把数组中保存的所有管理定时器对象的shared_ptr释放掉
        }
};

class Test {
    public:
        Test() {std::cout << "构造" << std::endl;}
        ~Test() {std::cout << "析构" << std::endl;}
};

void DelTest(Test *t) {
    delete t;
}

int main()
{
    TimerWheel tw;

    Test *t = new Test();

    tw.TimerAdd(888, 5, std::bind(DelTest, t));

    for(int i = 0; i < 5; i++) {
        sleep(1);
        tw.TimerRefresh(888);//刷新定时任务
        tw.RunTimerTask();//向后移动秒针
        std::cout << "刷新了一下定时任务，重新需要5s中后才会销毁\n";
    }
    tw.TimerCancel(888);
    while(1) {
        sleep(1);
        std::cout << "-------------------\n";
        tw.RunTimerTask();//向后移动秒针
    }
    return 0;
}

