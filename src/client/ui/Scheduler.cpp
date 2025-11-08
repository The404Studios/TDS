#include "Scheduler.h"
#include <algorithm>

Scheduler::Scheduler() : currentTime(0.0f) {
}

std::shared_ptr<ScheduledTask> Scheduler::scheduleTask(std::function<void()> callback, float delay) {
    auto task = std::make_shared<ScheduledTask>(callback, delay);
    task->nextExecutionTime = currentTime + delay;
    tasks.push_back(task);
    return task;
}

std::shared_ptr<ScheduledTask> Scheduler::scheduleRepeating(std::function<void()> callback, float interval) {
    auto task = std::make_shared<ScheduledTask>(callback, interval, interval);
    task->nextExecutionTime = currentTime + interval;
    tasks.push_back(task);
    return task;
}

void Scheduler::scheduleNextFrame(std::function<void()> callback) {
    nextFrameTasks.push_back(callback);
}

void Scheduler::cancelTask(std::shared_ptr<ScheduledTask> task) {
    if (task) {
        task->cancelled = true;
    }
}

void Scheduler::update(float deltaTime) {
    currentTime += deltaTime;

    // Execute next frame tasks
    for (auto& callback : nextFrameTasks) {
        callback();
    }
    nextFrameTasks.clear();

    // Execute scheduled tasks
    std::vector<std::shared_ptr<ScheduledTask>> toRemove;

    for (auto& task : tasks) {
        if (task->cancelled) {
            toRemove.push_back(task);
            continue;
        }

        if (currentTime >= task->nextExecutionTime) {
            // Execute task
            task->callback();

            // Handle repeating tasks
            if (task->repeatInterval > 0.0f) {
                task->nextExecutionTime = currentTime + task->repeatInterval;
            } else {
                toRemove.push_back(task);
            }
        }
    }

    // Remove completed/cancelled tasks
    for (auto& task : toRemove) {
        tasks.erase(std::remove(tasks.begin(), tasks.end(), task), tasks.end());
    }
}

void Scheduler::clear() {
    tasks.clear();
    nextFrameTasks.clear();
}

size_t Scheduler::getPendingTaskCount() const {
    return tasks.size() + nextFrameTasks.size();
}
