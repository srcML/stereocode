// A qualified type name that ends with the name of the own class names another class, unless its qualifiers are those of the own class

namespace wire {
    class Request { };
}

namespace http {
    class Request {
        int m_size;
    public:
        // @stereotype set collaborator
        void copy(const wire::Request& other) { m_size = 1; }

        // @stereotype set
        void take(const http::Request& other) { m_size = 2; }

        // @stereotype set
        void keep(const Request& other) { m_size = 3; }

        Request* twin(const ::http::Request& other);
    };

    // A definition outside the class writes the qualifiers of the class
    // @stereotype property
    http::Request* http::Request::twin(const ::http::Request& other) { return m_size > 0 ? this : nullptr; }
}
