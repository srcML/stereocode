// The calls between the types of two files count alike in either order of the files, as data_member_calls_type_first shows

class Car;

class Engine {
    int m_rpm;
    Car* m_owner;
public:
    // @stereotype get
    int raw() const { return m_rpm; }

    // @stereotype property
    int rpm() const { return raw() / 2; }

    // @stereotype property collaborator
    int ownerSpeed() { return m_owner->speed() + 1; }
};
