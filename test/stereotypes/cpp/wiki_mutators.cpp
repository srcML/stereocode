// Method-Stereotypes wiki, Structural Mutators, C++ examples (HippoDraw), each in a minimal class

#include <string>
#include <vector>
using std::string;

class dataSource {
  string m_ds_name;
public:
  void setName(const string& name);
};

// @stereotype set
void dataSource::setName(const string& name){
  m_ds_name = name;
}

class ListTuple {
  std::vector<unsigned int> m_shape;
public:
  void setShape(std::vector<unsigned int>& shape);
};

// @stereotype set
void ListTuple::setShape(std::vector<unsigned int>& shape) {
  m_shape = shape;
}

class DataSource {
  std::string m_ds_name;
  bool m_is_null;
  std::vector<std::string> m_labels;
  std::vector<unsigned int> m_shape;
  std::string m_title;
public:
  void copyPrivate(const DataSource& other);
};

// @stereotype command
void DataSource::copyPrivate(const DataSource& other) {
  m_ds_name = other.m_ds_name;
  m_is_null = other.m_is_null;
  m_labels = other.m_labels;
  m_shape = other.m_shape;
  m_title = other.m_title;
}
