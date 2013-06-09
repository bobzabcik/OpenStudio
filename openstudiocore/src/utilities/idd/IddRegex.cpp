/**********************************************************************
*  Copyright (c) 2008-2013, Alliance for Sustainable Energy.  
*  All rights reserved.
*  
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License as published by the Free Software Foundation; either
*  version 2.1 of the License, or (at your option) any later version.
*  
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Lesser General Public License for more details.
*  
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**********************************************************************/

#include <utilities/idd/IddRegex.hpp>

namespace openstudio{
namespace iddRegex{

  /// name of the comment only object automatically added to the idd
  const std::string &commentOnlyObjectName(){
    const static std::string result("CommentOnly");
    return result;
  }

  /// text of the comment only object automatically added to the idd
  const std::string &commentOnlyObjectText(){
    const static std::string result("CommentOnly; ! Autogenerated comment only object.");
    return result;
  }

  /// Search for IDD version in line
  /// matches[1], version identifier
  const boost::regex &version(){
    const static boost::regex result("^!IDD_Version ([0-9\\.]+)", boost::regex_constants::optimize);
    return result; 
  }

  /// Search for IDD header, each line must start with '!', no preceeding whitespace
  /// matches[1], header
  const boost::regex &header(){
    const static boost::regex result("^(^!.*?^[^!])", boost::regex_constants::optimize);
    return result; 
  }

  /// Match comment only line
  /// matches[1], comment
  const boost::regex &commentOnlyLine(){
    const static boost::regex result("^[\\s\\t]*[!](.*)", boost::regex_constants::optimize);
    return result; 
  }

  /// Match content then comment
  /// matches[1], content
  /// matches[2], comment if any
  const boost::regex &contentAndCommentLine(){
    const static boost::regex result("^([^!]*)[!]?(.*)", boost::regex_constants::optimize);
    return result; 
  }

  /// Match group identifier
  /// matches[1], group name
  const boost::regex &group(){
    const static boost::regex result("^[\\\\][gG]roup(.*)", boost::regex_constants::optimize);
    return result; 
  }

  /// Match include-file identifier
  /// matches[1], incluced Idd file name
  const boost::regex &includeFile() {
    const static boost::regex result("^[\\\\][iI]nclude-[fF]ile(.*)", boost::regex_constants::optimize);
    return result;
  }

  /// Match remove-object identifier
  /// matches[1], object in included Idd file that should not be included in this file
  const boost::regex &removeObject() {
    const static boost::regex result("^[\\\\][rR]emove-[oO]bject(.*)", boost::regex_constants::optimize);
    return result;
  }

  /// Match line with either a ',' or a ';' that are not preceeded by '!'
  /// matches[1], before seperator
  /// matches[2], after seperator
  const boost::regex &line(){
    const static boost::regex result("^([^!]*?)[,;](.*)", boost::regex_constants::optimize);
    return result;
  }

  /// Match an object memo property
  /// matches[1], memo text
  const boost::regex &memoProperty(){
    const static boost::regex result("^[mM]emo(.*)", boost::regex_constants::optimize);
    return result;
  }

  /// Match an object note property
  /// matches[1], note text
  const boost::regex &noteProperty(){
    const static boost::regex result("^[nN]ote(.*)", boost::regex_constants::optimize);
    return result;
  }

  /// Match an object with no fields in the idd
  /// matches[1], before seperator
  /// matches[2], after seperator
  const boost::regex &objectNoFields(){
    const static boost::regex result("^([^!^,]*?)[;](.*)", boost::regex_constants::optimize);
    return result;
  }

  /// Match an object with one or more fields in the idd
  /// matches[1], object text
  /// matches[2], field(s) text
  const boost::regex &objectAndFields(){
    const static boost::regex result("^(.*?[,].*?)([AN][0-9]+[\\s\\t]*[,;].*)", boost::regex_constants::optimize);
    return result;
  }

  /// Match an object unique property
  const boost::regex &uniqueProperty(){
    const static boost::regex result("^[uU]nique-[oO]bject", boost::regex_constants::optimize);
    return result;
  }

  /// Match an object required property
  const boost::regex &requiredObjectProperty(){
    const static boost::regex result("^[rR]equired-[oO]bject", boost::regex_constants::optimize);
    return result;
  }

  /// Match an object obsolete property
  /// matches[1], reason for obsolete
  const boost::regex &obsoleteProperty(){
    const static boost::regex result("^[oO]bsolete(.*)", boost::regex_constants::optimize);
    return result;
  }

  const boost::regex &hasurlProperty()
  {
    const static boost::regex result("^(URL-[Oo]bject|url-[Oo]bject)", boost::regex_constants::optimize);
    return result;
  }

  /// Match an object extensible property
  /// matches[1], number of last fields to extend
  const boost::regex &extensibleProperty(){
    const static boost::regex result("^[eE]xtensible[\\s\\t]*:[\\s\\t]*([1-9][0-9]*)", boost::regex_constants::optimize);
    return result;
  }

  /// Match an object format property
  /// matches[1], format text
  const boost::regex &formatProperty(){
    const static boost::regex result("^[fF]ormat([^!]*)", boost::regex_constants::optimize);
    return result;
  }

  /// Match an object min fields property
  /// matches[1], min number of fields
  const boost::regex &minFieldsProperty(){
    const static boost::regex result("^[mM]in-[fF]ields[\\s\\t]*([0-9]+)", boost::regex_constants::optimize);
    return result;
  }

  const boost::regex &maxFieldsProperty() {
    const static boost::regex result("^[mM]ax-[fF]ields[\\s\\t]*([0-9]+)", boost::regex_constants::optimize);
    return result;
  }

  /// Match a field declaration in the idd
  /// A or N, then one or more numbers then white space and then a ',' or ';'
  /// matches[1], alpha or numeric indicator
  /// matches[2], alpha or numeric number
  /// matches[3], after seperator
  const boost::regex &field(){
    const static boost::regex result("^[\\s\\t]*?([AN])([0-9]+)[\\s\\t]*[,;](.*)", boost::regex_constants::optimize);
    return result;
  }

  /// Match the closing field in an idd object
  /// matches[1], all previous text
  /// matches[2], the last field
  const boost::regex &closingField(){
    const static boost::regex result("(.*)([AN][0-9]+[\\s\\t]*[;].*?)$", boost::regex_constants::optimize);
    return result;
  }

  /// Match the last field declaration in a string, may or may not be the closing field
  /// matches[1], all previous text
  /// matches[2], the last field
  const boost::regex &lastField(){
    const static boost::regex result("(.*)([AN][0-9]+[\\s\\t]*[,;].*)$", boost::regex_constants::optimize);
    return result;
  }

  /// Match a field name 
  /// matches[1], the field name
  const boost::regex &name(){
    const static boost::regex result("[\\\\][fF]ield([^\\\\^!]*)", boost::regex_constants::optimize);
    return result;
  }

  /// Match a field field name property
  /// matches[1], the field name
  const boost::regex &nameProperty(){
    const static boost::regex result("^[fF]ield([^!]*)", boost::regex_constants::optimize);
    return result;
  }

  /// Match a field required property
  const boost::regex &requiredFieldProperty(){
    const static boost::regex result("^[rR]equired-[fF]ield", boost::regex_constants::optimize);
    return result;
  }

  /// Match a field autosizable property
  const boost::regex &autosizableProperty(){
    const static boost::regex result("^[aA]utosizable", boost::regex_constants::optimize);
    return result;
  }

  /// Match a field autocalculatable property
  const boost::regex &autocalculatableProperty(){
    const static boost::regex result("^[aA]utocalculatable", boost::regex_constants::optimize);
    return result;
  }

  /// Match a field retaincase property
  const boost::regex &retaincaseProperty(){
    const static boost::regex result("^[rR]etaincase", boost::regex_constants::optimize);
    return result;
  }

  /// Match a field units property
  /// matches[1], the field units
  const boost::regex &unitsProperty(){
    const static boost::regex result("^[uU]nits([^!]*)", boost::regex_constants::optimize);
    return result;
  }

  /// Match a field ip units property
  /// matches[1], the field ip units
  const boost::regex &ipUnitsProperty(){
    const static boost::regex result("^[iI][pP]-[uU]nits([^!]*)", boost::regex_constants::optimize);
    return result;
  }

  /// Match a field exclusive minimum property
  /// matches[1], the field exclusive minimum
  const boost::regex &minExclusiveProperty(){
    const static boost::regex result("^[mM]inimum[\\s\\t]*[>]([^!]*)", boost::regex_constants::optimize);
    return result;
  }

  /// Match a field inclusive minimum property
  /// matches[1], the field inclusive minimum
  const boost::regex &minInclusiveProperty(){
    const static boost::regex result("^[mM]inimum([^>!]*)", boost::regex_constants::optimize);
    return result;
  }

  /// Match a field exclusive maximum property
  /// matches[1], the field exclusive maximum
  const boost::regex &maxExclusiveProperty(){
    const static boost::regex result("^[mM]aximum[\\s\\t]*[<]([^!]*)", boost::regex_constants::optimize);
    return result;
  }

  /// Match a field inclusive maximum property
  /// matches[1], the field inclusive maximum
  const boost::regex &maxInclusiveProperty(){
    const static boost::regex result("^[mM]aximum([^<!]*)", boost::regex_constants::optimize);
    return result;
  }

  /// Match a field deprecated property
  /// matches[1], reason for deprecated
  const boost::regex &deprecatedProperty(){
    const static boost::regex result("^[dD]eprecated(.*)", boost::regex_constants::optimize);
    return result;
  }

  /// Match a field default property
  /// matches[1], default value 
  const boost::regex &defaultProperty(){
    const static boost::regex result("^[dD]efault([^!]*)", boost::regex_constants::optimize);
    return result;
  }

  /// Match a field default property with either autocalculate or autosize
  const boost::regex &automaticDefault(){
    const static boost::regex result(".*(autocalculate|autosize).*", boost::regex_constants::icase | boost::regex_constants::optimize);
    return result;
  }

  /// Match a field type property
  /// matches[1], type
  const boost::regex &typeProperty(){
    const static boost::regex result("^type[\\s\\t]*(integer|real|alpha|choice|node|object-list|external-list|url|handle)",
      boost::regbase::normal | boost::regbase::icase | boost::regex_constants::optimize);
    return result;
  }

  /// Match a field key property
  /// matches[1], key value 
  const boost::regex &keyProperty(){
    const static boost::regex result("^[kK]ey(.*)", boost::regex_constants::optimize);
    return result;
  }

  /// Match a field object-list property
  /// matches[1], object-list value 
  const boost::regex &objectListProperty(){
    const static boost::regex result("^[oO]bject-[lL]ist([^!]*)", boost::regex_constants::optimize);
    return result;
  }

  const boost::regex &externalListProperty() {
    const static boost::regex result("^[eE]xternal-[lL]ist([^!]*)", boost::regex_constants::optimize);
    return result;
  }

  /// Match a field reference property
  /// matches[1], reference value 
  const boost::regex &referenceProperty(){
    const static boost::regex result("^[rR]eference([^!]*)", boost::regex_constants::optimize);
    return result;
  }

  /// Match begin extensible
  const boost::regex &beginExtensible(){
    const static boost::regex result("[\\\\][bB]egin-[eE]xtensible", boost::regex_constants::optimize);
    return result;
  }

  /// Match begin extensible
  const boost::regex &beginExtensibleProperty(){
    const static boost::regex result("^[bB]egin-[eE]xtensible", boost::regex_constants::optimize);
    return result;
  }

  /// Match a field or object level comment
  /// matches[1], after '\' until next '\'
  /// matches[2], after second '\' (may be empty)
  const boost::regex &metaDataComment(){
    const static boost::regex result("^[\\s\\t]*?[\\\\]([^\\\\]*)(.*)", boost::regex_constants::optimize);
    return result;
  }

  const boost::regex &versionObjectName() {
    const static boost::regex result(".*[vV]ersion.*", boost::regex_constants::optimize);
    return result;
  }

} // iddRegex
} // openstudio

