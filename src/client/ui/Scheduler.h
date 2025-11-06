#pragma once

#include <functional>
#include <queue>
#include <vector>
#include <memory>
#include <chrono>

// Scheduled task
struct ScheduledTask {
    std::function<void()> callback;
    float delay;              // Delay in seconds before execution
    float repeatInterval;     // If > 0, task repeats with this interval
    float nextExecutionTime;  // Time when task should execute
    bool cancelled;

    ScheduledTask(std::function<void()> cb, float delay, float repeat = 0.0f)
        : callback(cb), delay(delay), repeatInterval(repeat),
          nextExecutionTime(delay), cancelled(false) {}
};

// Scheduler for delayed and repeated task execution
class Scheduler {
public:
    Scheduler();

    // Schedule a task to run after delay (in seconds)
    std::shared_ptr<ScheduledTask> scheduleTask(std::function<void()> callback, float delay);

    // Schedule a repeating task
    std::shared_ptr<ScheduledTask> scheduleRepeating(std::function<void()> callback, float interval);

    // Schedule a task to run next frame
    void scheduleNextFrame(std::function<void()> callback);

    // Cancel a task
    void cancelTask(std::shared_ptr<ScheduledTask> task);

    // Update scheduler (call every frame)
    void update(float deltaTime);

    // Clear all tasks
    void clear();

    // Get number of pending tasks
    size_t getPendingTaskCount() const;

private:
    std::vector<std::shared_ptr<ScheduledTask>> tasks;
    std::vector<std::function<void()>> nextFrameTasks;
    float currentTime;
};
