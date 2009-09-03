/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, version 1.0 beta 4      *
*                (c) 2006-2009 MGH, INRIA, USTL, UJF, CNRS                    *
*                                                                             *
* This library is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This library is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this library; if not, write to the Free Software Foundation,     *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.          *
*******************************************************************************
*                              SOFA :: Framework                              *
*                                                                             *
* Authors: M. Adam, J. Allard, B. Andre, P-J. Bensoussan, S. Cotin, C. Duriez,*
* H. Delingette, F. Falipou, F. Faure, S. Fonteneau, L. Heigeas, C. Mendoza,  *
* M. Nesme, P. Neumann, J-P. de la Plata Alcade, F. Poyer and F. Roy          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#include <sofa/core/objectmodel/BaseObject.h>
#include <sofa/core/objectmodel/BaseContext.h>
#include <sofa/core/objectmodel/Event.h>
#include <sofa/core/objectmodel/KeypressedEvent.h>
#include <sofa/core/componentmodel/topology/Topology.h>
#include <sofa/helper/TagFactory.h>
#include <iostream>

using std::cerr;
using std::endl;

namespace sofa
{

namespace core
{

namespace objectmodel
{

BaseObject::BaseObject()
    : Base()
    , f_listening(initData( &f_listening, false, "listening", "if true, handle the events, otherwise ignore the events"))
    , f_tags(initData( &f_tags, "tags", "list of the subsets the objet belongs to"))
    , context_(NULL)
/*        , m_isListening(false)
        , m_printLog(false)*/
{
}

BaseObject::~BaseObject()
{}

void BaseObject::parse( BaseObjectDescription* arg )
{
    std::vector< std::string > attributeList;
    arg->getAttributeList(attributeList);
    for (unsigned int i=0; i<attributeList.size(); ++i)
    {
#ifdef SOFA_DEV
        if (attributeList[i] == "src")
        {
            // Parse attribute 'src' for new MeshLoader architecture.
            const char* val = arg->getAttribute(attributeList[i]);
            std::string valueString(val);

            if(!val)
            {
                serr<<"ERROR: Missing argument for 'src' attribute in object: "<< this->getName() << sendl;
                break;
            }

            if (valueString[0] != '@')
            {
                serr<<"ERROR: 'src' attribute value should be a link using '@' in object "<< this->getName() << sendl;
                break;
            }

            BaseObject* obj = this;
            BaseObject* loader = NULL;

            std::size_t posAt = valueString.rfind('@');
            if (posAt == std::string::npos) posAt = 0;
            std::string objectName;

            objectName = valueString.substr(posAt+1);
            loader = getContext()->get<BaseObject>(objectName);

            std::map < std::string, BaseData*> dataLoaderMap;
            std::map < std::string, BaseData*>::iterator it_map;

            //std::cout << "- objet en cours: " << obj->getName() << std::endl;
            //std::cout << "- loader associe: " << objectName << std::endl;

            for (unsigned int j = 0; j<loader->m_fieldVec.size(); ++j)
            {
                dataLoaderMap.insert (std::pair<std::string, BaseData*> (loader->m_fieldVec[j].first, loader->m_fieldVec[j].second));
                //std::cout << "Data loader: " << j << " => " << loader->m_fieldVec[j].first << std::endl;
            }

            for (unsigned int j = 0; j<attributeList.size(); ++j)
            {
                //std::cout << "Data Object filled: " << attributeList[j] << std::endl;
                it_map = dataLoaderMap.find (attributeList[j]);
                if (it_map != dataLoaderMap.end())
                    dataLoaderMap.erase (it_map);
            }

            // -- Temporary patch, using exceptions. TODO: use a flag to set Data not to be automatically linked. --
            //{
            it_map = dataLoaderMap.find ("name");
            if (it_map != dataLoaderMap.end())
                dataLoaderMap.erase (it_map);

            it_map = dataLoaderMap.find ("type");
            if (it_map != dataLoaderMap.end())
                dataLoaderMap.erase (it_map);

            it_map = dataLoaderMap.find ("filename");
            if (it_map != dataLoaderMap.end())
                dataLoaderMap.erase (it_map);

            it_map = dataLoaderMap.find ("tags");
            if (it_map != dataLoaderMap.end())
                dataLoaderMap.erase (it_map);

            it_map = dataLoaderMap.find ("printLog");
            if (it_map != dataLoaderMap.end())
                dataLoaderMap.erase (it_map);

            it_map = dataLoaderMap.find ("listening");
            if (it_map != dataLoaderMap.end())
                dataLoaderMap.erase (it_map);
            //}


            for (it_map =dataLoaderMap.begin(); it_map != dataLoaderMap.end(); ++it_map)
            {
                BaseData* Data = obj->findField( (*it_map).first );

                if (Data != NULL)
                {
                    //serr<<"Adding link for: " << (*it_map).first << sendl;
                    Data->addInput( (*it_map).second);
                }
            }

        }
#endif //SOFA_DEV


        std::vector< BaseData* > dataModif = findGlobalField(attributeList[i]);
        for (unsigned int d=0; d<dataModif.size(); ++d)
        {
            const char* val = arg->getAttribute(attributeList[i]);
            if (val)
            {
                std::string valueString(val);

                /* test if data is a link */
                if (valueString[0] == '@')
                {
                    std::size_t posPath = valueString.rfind('/');
                    if (posPath == std::string::npos) posPath = 0;
                    std::size_t posDot = valueString.rfind('.');
                    std::string objectName;
                    std::string dataName;

                    BaseObject* obj = NULL;

                    /* if '.' not found, try to find the data in the current object */
                    if (posPath == 0 && posDot == std::string::npos)
                    {
                        obj = this;
                        objectName = this->getName();
                        dataName = valueString;
                    }
                    else
                    {
                        if (posDot == std::string::npos) // no data specified, look for one with the same name
                        {
                            objectName = valueString;
                            dataName = attributeList[i];
                        }
                        else
                        {
                            objectName = valueString.substr(1,posDot-1);
                            dataName = valueString.substr(posDot+1);
                        }

                        if (objectName[0] == '[')
                        {
                            if (objectName[objectName.size()-1] != ']')
                            {
                                serr<<"ERROR: Missing ']' in at the end of "<< objectName << sendl;
                                break;
                            }

                            objectName = objectName.substr(1, objectName.size()-2);

                            if (objectName.empty())
                            {
                                serr<<"ERROR: Missing object level between [] in : " << val << sendl;
                                break;
                            }

                            int objectLevel = atoi(objectName.c_str());
                            helper::vector<BaseObject*> objects;
                            getContext()->get<BaseObject>(&objects, BaseContext::Local);

                            helper::vector<BaseObject*>::iterator it;

                            for(it = objects.begin(); it != objects.end(); ++it)
                            {
                                if ((*it) == this)
                                {
                                    it += objectLevel;
                                    if ((*it) != NULL)
                                    {
                                        obj = (*it);
                                    }
                                    break;
                                }
                            }
                        }
                        else
                        {
                            obj = getContext()->get<BaseObject>(objectName);
                        }

                        if (obj == NULL)
                        {
                            serr<<"could not find object for option "<< attributeList[i] <<": " << objectName << sendl;
                            break;
                        }
                    }

                    BaseData* parentData = obj->findField(dataName);

                    if (parentData == NULL)
                    {
                        serr<<"could not read value for option "<< attributeList[i] <<": " << val << sendl;
                        break;
                    }

                    /* set parent value to the child */
                    if (!dataModif[d]->setParentValue(parentData))
                    {
                        serr<<"could not copy value from parent Data "<< valueString << ". Incompatible Data types" << sendl;
                        break;
                    }
                    parentData->addOutput(dataModif[d]);
                    /* children Data can be modified changing the parent Data value */
                    dataModif[d]->setReadOnly(true);
                    break;
                }

                if( !(dataModif[d]->read( valueString ))) serr<<"could not read value for option "<< attributeList[i] <<": " << val << sendl;
            }
        }
    }
}

void BaseObject::setContext(BaseContext* n)
{
    context_ = n;
}

const BaseContext* BaseObject::getContext() const
{
    //return (context_==NULL)?BaseContext::getDefault():context_;
    return context_;
}

BaseContext* BaseObject::getContext()
{
    return (context_==NULL)?BaseContext::getDefault():context_;
    //return context_;
}

void BaseObject::init()
{
}

void BaseObject::bwdInit()
{
}

/// Update method called when variables used in precomputation are modified.
void BaseObject::reinit()
{
    //sout<<"WARNING: the reinit method of the object "<<this->getName()<<" does nothing."<<sendl;
}

/// Save the initial state for later uses in reset()
void BaseObject::storeResetState()
{ }

/// Reset to initial state
void BaseObject::reset()
{ }

void BaseObject::writeState( std::ostream& )
{ }

/// Called just before deleting this object
/// Any object in the tree bellow this object that are to be removed will be removed only after this call,
/// so any references this object holds should still be valid.
void BaseObject::cleanup()
{ }

/// Handle an event
void BaseObject::handleEvent( Event* /*e*/ )
{
    /*
    serr<<"BaseObject "<<getName()<<" ("<<getTypeName()<<") gets an event"<<sendl;
    if( KeypressedEvent* ke = dynamic_cast<KeypressedEvent*>( e ) )
    {
        serr<<"BaseObject "<<getName()<<" gets a key event: "<<ke->getKey()<<sendl;
    }
    */
}

/// Handle topological Changes from a given Topology
void BaseObject::handleTopologyChange(core::componentmodel::topology::Topology* t)
{
    if (t == this->getContext()->getTopology())
    {
        //	sout << getClassName() << " " << getName() << " processing topology changes from " << t->getName() << sendl;
        handleTopologyChange();
    }
}

/// Handle state Changes from a given Topology
void BaseObject::handleStateChange(core::componentmodel::topology::Topology* t)
{
    if (t == this->getContext()->getTopology())
        handleStateChange();
}

// void BaseObject::setListening( bool b )
// {
//     m_isListening = b;
// }
//
// bool BaseObject::isListening() const
// {
//     return m_isListening;
// }
//
// BaseObject* BaseObject::setPrintLog( bool b )
// {
//     m_printLog = b;
//     return this;
// }
//
// bool BaseObject::printLog() const
// {
//     return m_printLog;
// }

double BaseObject::getTime() const
{
    return getContext()->getTime();
}

bool BaseObject::hasTag(Tag t) const
{
    return (f_tags.getValue().count( t ) > 0 );
}


void BaseObject::addTag(Tag t)
{
    f_tags.beginEdit()->insert(t);
    f_tags.endEdit();
}

void BaseObject::removeTag(Tag t)
{
    f_tags.beginEdit()->erase(t);
    f_tags.endEdit();
}

#ifdef WIN32
__declspec(thread)
#elif !defined(__APPLE__)
__thread
#endif
bool tls_prefetching = false;

void BaseObject::setPrefetching(bool b)
{
    tls_prefetching = b;
}

bool BaseObject::isPrefetching()
{
    return tls_prefetching;
}


} // namespace objectmodel

} // namespace core

} // namespace sofa

