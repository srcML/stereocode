// The calls between the types of two files count alike in either order of the files, as data_member_calls_type_first shows

class Car {
    Engine m_engine;
public:
    // @stereotype property collaborator
    int speed() { return m_engine.rpm() / 100; }
};
