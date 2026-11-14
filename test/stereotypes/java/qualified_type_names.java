// A qualified type name that ends with the name of the own class names another class, unless its qualifiers are those of the own class

interface Connection {
    interface Request { }
}

class HttpConnection {
    static class Request implements Connection.Request {
        private int size;

        // @stereotype set collaborator
        void copy(Connection.Request other) { size = 1; }

        // @stereotype set
        void take(HttpConnection.Request other) { size = 2; }

        // @stereotype set
        void keep(Request other) { size = 3; }
    }
}
