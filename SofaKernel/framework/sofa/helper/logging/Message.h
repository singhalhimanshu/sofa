/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, development version     *
*                (c) 2006-2017 INRIA, USTL, UJF, CNRS, MGH                    *
*                                                                             *
* This program is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This program is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this program. If not, see <http://www.gnu.org/licenses/>.        *
*******************************************************************************
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
/*****************************************************************************
* User of this library should read the documentation
* in the TextMessaging.h file.
*****************************************************************************/
#ifndef MESSAGE_H
#define MESSAGE_H

#include <iostream>
#include <string>
#include <cstring>
#include <sofa/helper/helper.h>
#include <sstream>
#include <set>

#include <boost/shared_ptr.hpp>

namespace sofa
{

namespace core
{
    namespace objectmodel
{
class Base; // forward declaration
} // namespace objectmodel
} // namespace core
}

namespace sofa
{

namespace helper
{

namespace logging
{

inline bool notMuted(const std::string&){ return true; }

static const char * s_unknownFile = "unknown-file";

/// To keep a trace (file,line) from where the message have been created
/// The filename must be a valid pointer throughoug the message processing
/// If this cannot be guaranteed then use the FileInfoOwningFilename class
/// instead.
struct FileInfo
{
    typedef boost::shared_ptr<FileInfo> SPtr;

    const char *filename {nullptr};
    int line             {0};
    FileInfo(const char *f, int l): filename(f), line(l) {}

protected:
    FileInfo(): filename(s_unknownFile), line(0) {}
};

/// To keep a trace (file,line) from where the message have been created
struct FileInfoOwningFilename : public FileInfo
{
    FileInfoOwningFilename(const char *f, int l) {
        char *tmp  = new char[strlen(f)+1] ;
        strcpy(tmp, f) ;
        filename = tmp ;
        line = l ;
    }

    FileInfoOwningFilename(const std::string& f, int l) {
        char *tmp  = new char[f.size()+1] ;
        strcpy(tmp, f.c_str()) ;
        filename = tmp ;
        line = l ;
    }

    ~FileInfoOwningFilename(){
        if(filename)
            delete filename ;
    }
};


/// To keep track component informations associated with a message.
struct SOFA_HELPER_API ComponentInfo
{
public:
    ComponentInfo(){}
    ComponentInfo(const std::string& name) { m_sender = name ; }
    virtual ~ComponentInfo() {}

    virtual std::ostream& toStream(std::ostream& out) const {
        out << m_sender ;
        return out ;
    }

    const std::string& sender(){ return m_sender; }

    friend std::ostream& operator<<(std::ostream& out, const ComponentInfo& nfo) ;
    friend std::ostream& operator<<(std::ostream& out, const ComponentInfo* nfo) ;

    typedef boost::shared_ptr<ComponentInfo> SPtr;
protected:
    std::string m_sender ;
};


static FileInfo::SPtr EmptyFileInfo(new FileInfo(s_unknownFile, 0)) ;

#define SOFA_FILE_INFO sofa::helper::logging::FileInfo::SPtr(new sofa::helper::logging::FileInfo(__FILE__, __LINE__))
#define SOFA_FILE_INFO2(file,line) sofa::helper::logging::FileInfo::SPtr(new sofa::helper::logging::FileInfoOwningFilename(file,line))

class SOFA_HELPER_API Message
{
public:

    /// possible levels of messages (ordered)
    enum Type {Info=0, Advice, Deprecated, Warning, Error, Fatal, TEmpty, TypeCount};
    typedef std::set<Type> TypeSet;
    static TypeSet AnyTypes ;

    /// class of messages
    enum Class {Dev, Runtime, Log, CEmpty, ClassCount};

    Message() {}
    Message( const Message& msg );
    Message(Class mclass, Type type,
            const ComponentInfo::SPtr& = ComponentInfo::SPtr(),
            const FileInfo::SPtr& fileInfo = EmptyFileInfo) ;

    Message& operator=( const Message& msg );

    const FileInfo::SPtr&       fileInfo() const { return m_fileInfo; }
    const ComponentInfo::SPtr&  componentInfo() const { return m_componentinfo ; }
    const std::stringstream& message() const  { return m_stream; }
    Class                    context() const  { return m_class; }
    Type                     type() const     { return m_type; }
    const std::string&       sender() const   { return m_componentinfo->sender(); }

    const std::string messageAsString() const  { return m_stream.str(); }


    bool empty() const;

    template<class T>
    Message& operator<<(const T &x)
    {
        m_stream << x;
        return *this;
    }

    static Message emptyMsg ;

protected:
    ComponentInfo::SPtr m_componentinfo; /// a trace (name, path) from whom has emitted this message.
    FileInfo::SPtr      m_fileInfo; ///< a trace (file,line) from where the message have been created
    std::stringstream   m_stream; ///< the actual message
    Class               m_class; ///< who is the attender of the message (developers or users)?
    Type                m_type; ///< the message level
    int                 m_id; ///< should it be stored here or in the handler that needs it?

};

SOFA_HELPER_API std::ostream& operator<< (std::ostream&, const Message&) ;

} // logging
} // helper
} // sofa


#endif // MESSAGE_H
