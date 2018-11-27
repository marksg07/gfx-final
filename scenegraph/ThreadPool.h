#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>

typedef std::function<void()> Job;

class ThreadPool
{
public:
    ThreadPool()
        : ThreadPool(std::thread::hardware_concurrency())
    {
    }

    ThreadPool(size_t threads)
    {
        if (threads == 0)
        {
            threads = 4;
        }

        m_runningJobs = 0;

        for(size_t i = 0; i < threads; i++)
        {
            m_threads.push_back(std::make_unique<std::thread>([this]() {
                this->jobHandler();
            }));
        }

        m_stopped = false;
    }

    ~ThreadPool()
    {
        m_stopped = true;
        m_cond.notify_all();

        for(size_t i = 0; i < m_threads.size(); i++)
        {
            m_threads[i]->join();
        }

        m_done.notify_all();
    }

    void addJob(Job job)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_jobs.push(job);

        m_cond.notify_one();
    }

    void wait()
    {
        std::unique_lock<std::mutex> lock(m_mtx);

        m_done.wait(lock);
    }

private:

    void jobHandler()
    {
        while(!m_stopped)
        {
            std::unique_lock<std::mutex> lock(m_mtx);

            while(m_jobs.size() == 0)
            {
                if (m_runningJobs == 0)
                {
                    m_done.notify_all();
                }

                if (m_stopped)
                {
                    return;
                }

                m_cond.wait(lock);
            }

            Job j = m_jobs.front();
            m_jobs.pop();

            m_runningJobs++;

            lock.unlock();

            j();
            m_runningJobs--;
        }
    }

    std::queue<Job> m_jobs;

    std::atomic<int> m_runningJobs;

    std::atomic<bool> m_stopped;

    std::vector<std::unique_ptr<std::thread>> m_threads;

    std::condition_variable m_done;

    std::condition_variable m_cond;

    std::mutex m_mtx;

};

#endif // THREADPOOL_H
