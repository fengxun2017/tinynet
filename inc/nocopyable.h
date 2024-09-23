
namespace tinynet
{
/* What kind of classes should be non-copyable?
 * Resource management classes: For example, classes that manage dynamic memory, file handles, or network connections. 
 *                              These classes typically allocate resources in the constructor and release resources in the destructor. 
 *                              If copying is allowed, it may result in duplicate release of resources or memory leaks.
 * Singleton pattern classes: Singleton patterns ensure that there is only one instance of a class. z
 *                              If copying is allowed, it destroys the uniqueness of the singleton pattern.
 * Classes that are tightly coupled to hardware or system resources: for example, classes that manage threads, mutexes, or other synchronization mechanisms. 
 *                              Copies of these classes may result in contention or deadlocks for system resources.
 */
class NonCopyable {
public:
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};

} // tinynet