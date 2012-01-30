#ifndef __ENVIRE_EVENTHANDLER__
#define __ENVIRE_EVENTHANDLER__

#include <envire/Core.hpp>
#include <envire/core/EventSource.hpp>
#include <envire/core/Event.hpp>
#include <boost/thread/mutex.hpp>

namespace envire
{

/** An EventHandler can be registered with an environment, and is then called,
 * whenever an new Event is generated within the Environment
 */
class EventHandler
{
public:
    virtual void handle( const Event& message ) = 0;
};

/** Event dispatcher interface class. Override the callbacks methods that you
 * are interested in.
 */
class EventDispatcher
{
public:
    static void dispatch( event::Type type, event::Operation operation, EnvironmentItem* a, EnvironmentItem* b, EventDispatcher* dispatch );

    virtual void itemAttached(EnvironmentItem *item);
    virtual void itemDetached(EnvironmentItem *item);
    virtual void childAdded(FrameNode* parent, FrameNode* child);
    virtual void childAdded(Layer* parent, Layer* child);

    virtual void frameNodeSet(CartesianMap* map, FrameNode* node);
    virtual void frameNodeDetached(CartesianMap* map, FrameNode* node);

    virtual void childRemoved(FrameNode* parent, FrameNode* child);
    virtual void childRemoved(Layer* parent, Layer* child);

    virtual void setRootNode(FrameNode *root);
    virtual void removeRootNode(FrameNode *root);

    virtual void itemModified(EnvironmentItem *item);
};

class EventFilter
{
public:
    virtual bool filter( envire::Event const& ) = 0;
};

/** Convenience class, that performs the dispatching of particular event
 * types into callback methods.
 */
class EventListener : public EventHandler, public EventDispatcher
{
public:
    EventListener();
    void handle( const Event& message );

    void setFilter( EventFilter *filter ) { this->filter = filter; }

protected:
    EventFilter *filter;
};

class EventQueue : public EventHandler
{
    std::list<Event> msgQueue;
    boost::mutex queueMutex;

public:
    void handle( const Event& message );
    void flush();
    virtual void process( const Event& message ) = 0;
};

/** EventHandler that will apply the handled events to the given Environment
 */
class EventProcessor : public EventQueue
{
    Environment *env;

public:
    EventProcessor( Environment *env );
    void process( const Event& message );
};

}

#endif
