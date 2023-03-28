#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

class IOxfEvent {
public:
    virtual ~IOxfEvent() { }
    virtual bool isTypeOf(const std::type_info& info) const = 0;
};

class MyEvent : public IOxfEvent {
public:
    virtual bool isTypeOf(const std::type_info& info) const {
        return typeid(MyEvent) == info;
    }
};

class IOxfReactive {
public:
    virtual ~IOxfReactive() { }
    virtual void handleEvent(IOxfEvent* event) = 0;
};

class MyReactiveObject : public IOxfReactive {
public:
    // Constructor
    MyReactiveObject() {
        // Create a new thread to handle events
        m_thread = std::thread(&MyReactiveObject::run, this);
    }

    // Destructor
    virtual ~MyReactiveObject() {
        //std::cout << __PRETTY_FUNCTION__<<std::endl;
        {
            m_running.store(false);
            m_cv.notify_all();
        }
        m_thread.join();
    }

    // send Event
    void sendEvent(IOxfEvent* event) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(event);
        m_cv.notify_one();
    }

protected:
    virtual void handleEvent(IOxfEvent* event) {
        // Process the event here
        std::cout << "Received event: "  << std::endl;
        // ...
        //delete event;
    }

private:
    // Event loop thread function
    void run() {
        while (m_running) {
            // Wait for an event to be added to the queue
            std::unique_lock<std::mutex> lock(m_mutex);
            //while (m_queue.empty()) {
              //  m_cv.wait(lock);
            //}
            m_cv.wait(lock,[this](){return !(m_queue.empty()) || !m_running.load();}); // this replaces (while condition)

            // Get the next event from the queue
            if (!(m_queue.empty())){
                IOxfEvent* event = m_queue.front();
                m_queue.pop();
                handleEvent(event);
            }

        }
    }

    // Member variables
    std::thread m_thread;
    std::queue<IOxfEvent*> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    //bool m_running = true;
    std::atomic_bool m_running = true;
};

int main() {
    // Create a new reactive object
    MyReactiveObject myObject;

    // Send an event to the reactive object
    myObject.sendEvent(new MyEvent());

    // Wait for the event to be processed
    std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
}
