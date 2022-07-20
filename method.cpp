//
//  method.cpp
//  
//
//  Created by jmaletic on July 20 2022.
//

#include "method.hpp"

methodModel::methodModel() : name(), parameters(),
                             parameterNames(), parameterTypes(),
                             srcML(), header(),
                             returnType(), constMethod(false),
                             retAttribute(false), attributesModified(0),
                             localVariables(), stereotype(NO_STEREOTYPE) {

};


methodModel::methodModel(const std::string& xml, const std::string& s, bool f) : methodModel() {
    srcML = xml;
    header = s;
    constMethod = f;
};
