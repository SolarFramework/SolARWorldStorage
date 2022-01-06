/**
* World Storage API
* API ensuring interoperability between an authoring tool and a World Storage service
*
* The version of the OpenAPI document: 0.0.1
* 
*
* NOTE: This class is auto generated by OpenAPI Generator (https://openapi-generator.tech).
* https://openapi-generator.tech
* Do not edit the class manually.
*/


#include "Transform3d.h"
#include "Helpers.h"

#include <sstream>

namespace org::openapitools::server::model
{

Transform3d::Transform3d()
{
    m_PayloadIsSet = false;
    m_Columnsize = 0;
    m_ColumnsizeIsSet = false;
    m_Rowsize = 0;
    m_RowsizeIsSet = false;
    
}

void Transform3d::validate() const
{
    std::stringstream msg;
    if (!validate(msg))
    {
        throw org::openapitools::server::helpers::ValidationException(msg.str());
    }
}

bool Transform3d::validate(std::stringstream& msg) const
{
    return validate(msg, "");
}

bool Transform3d::validate(std::stringstream& msg, const std::string& pathPrefix) const
{
    bool success = true;
    const std::string _pathPrefix = pathPrefix.empty() ? "Transform3d" : pathPrefix;

         
    if (payloadIsSet())
    {
        const std::vector<std::vector<double>>& value = m_Payload;
        const std::string currentValuePath = _pathPrefix + ".payload";
                
        
        { // Recursive validation of array elements
            const std::string oldValuePath = currentValuePath;
            int i = 0;
            for (const std::vector<double>& value : value)
            { 
                const std::string currentValuePath = oldValuePath + "[" + std::to_string(i) + "]";
                        
        
        { // Recursive validation of array elements
            const std::string oldValuePath = currentValuePath;
            int i = 0;
            for (const double& value : value)
            { 
                const std::string currentValuePath = oldValuePath + "[" + std::to_string(i) + "]";
                        
        
 
                i++;
            }
        }
 
                i++;
            }
        }

    }
            
    return success;
}

bool Transform3d::operator==(const Transform3d& rhs) const
{
    return
    
    
    
    ((!payloadIsSet() && !rhs.payloadIsSet()) || (payloadIsSet() && rhs.payloadIsSet() && getPayload() == rhs.getPayload())) &&
    
    
    ((!columnsizeIsSet() && !rhs.columnsizeIsSet()) || (columnsizeIsSet() && rhs.columnsizeIsSet() && getColumnsize() == rhs.getColumnsize())) &&
    
    
    ((!rowsizeIsSet() && !rhs.rowsizeIsSet()) || (rowsizeIsSet() && rhs.rowsizeIsSet() && getRowsize() == rhs.getRowsize()))
    
    ;
}

bool Transform3d::operator!=(const Transform3d& rhs) const
{
    return !(*this == rhs);
}

void to_json(nlohmann::json& j, const Transform3d& o)
{
    j = nlohmann::json();
    if(o.payloadIsSet() || !o.m_Payload.empty())
        j["payload"] = o.m_Payload;
    if(o.columnsizeIsSet())
        j["columnsize"] = o.m_Columnsize;
    if(o.rowsizeIsSet())
        j["rowsize"] = o.m_Rowsize;
    
}

void from_json(const nlohmann::json& j, Transform3d& o)
{
    if(j.find("payload") != j.end())
    {
        j.at("payload").get_to(o.m_Payload);
        o.m_PayloadIsSet = true;
    } 
    if(j.find("columnsize") != j.end())
    {
        j.at("columnsize").get_to(o.m_Columnsize);
        o.m_ColumnsizeIsSet = true;
    } 
    if(j.find("rowsize") != j.end())
    {
        j.at("rowsize").get_to(o.m_Rowsize);
        o.m_RowsizeIsSet = true;
    } 
    
}

std::vector<std::vector<double>> Transform3d::getPayload() const
{
    return m_Payload;
}
void Transform3d::setPayload(std::vector<std::vector<double>> const& value)
{
    m_Payload = value;
    m_PayloadIsSet = true;
}
bool Transform3d::payloadIsSet() const
{
    return m_PayloadIsSet;
}
void Transform3d::unsetPayload()
{
    m_PayloadIsSet = false;
}
int32_t Transform3d::getColumnsize() const
{
    return m_Columnsize;
}
void Transform3d::setColumnsize(int32_t const value)
{
    m_Columnsize = value;
    m_ColumnsizeIsSet = true;
}
bool Transform3d::columnsizeIsSet() const
{
    return m_ColumnsizeIsSet;
}
void Transform3d::unsetColumnsize()
{
    m_ColumnsizeIsSet = false;
}
int32_t Transform3d::getRowsize() const
{
    return m_Rowsize;
}
void Transform3d::setRowsize(int32_t const value)
{
    m_Rowsize = value;
    m_RowsizeIsSet = true;
}
bool Transform3d::rowsizeIsSet() const
{
    return m_RowsizeIsSet;
}
void Transform3d::unsetRowsize()
{
    m_RowsizeIsSet = false;
}


} // namespace org::openapitools::server::model
