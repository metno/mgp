/*
    MET-API

    Copyright (C) 2014 met.no
    Contact information:
    Norwegian Meteorological Institute
    Box 43 Blindern
    0313 OSLO
    NORWAY
    E-mail: met-api@met.no

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
    MA 02110-1301, USA
*/

package no.met.kdvh

import play.Logger
import com.github.nscala_time.time.Imports._

//// --- BEGIN testing ------------
//import play.api.libs.ws._
//import scala.concurrent._
//import play.api.Play.current
//import ExecutionContext.Implicits.global
//// --- END testing ------------

//$COVERAGE-OFF$Not testing database queries

/**
 * Concrete implementation of KdvhAccess class, used in non-production mode.
 */
class KdvhAccessNonProd extends KdvhAccess {
  override def getData(stationId: Int, obstime: Seq[Interval], elements: Seq[String], withQuality: Boolean): Seq[KdvhQueryResult] = {
    List[KdvhQueryResult]() // for now, just return nothing
  }

  // These names are not fixed yet
  private val translations = Map(
    "precipitation_amount" -> "RR_24",
    "min_air_temperature" -> "TAN",
    "max_air_temperature" -> "TAX",
    "air_temperature" -> "TA")
  private val reverseTranslations = translations map ((entry) => (entry._2, entry._1)) toMap // ### is toMap needed here?

  override def toKdvhElemName(apiElemName: String): String = {

//// --- BEGIN testing ------------
//    val request: WSRequestHolder = WS.url("http://eklima.met.no/metdata/MetDataService")
//      .withQueryString(
//        ("invoke", "getTimeserieTypesProperties"),
//        ("language", ""),
//        ("timeserieTypes", ""))
//
//    val response: Future[WSResponse] = request.get()
//
//    response onComplete {
//      case scala.util.Success(x) => println("success... x.body: " + x.body) // ### fix compilation error when removing 'scala.util.' qualifier
//      case scala.util.Failure(x) => println("failure: " + x.getMessage) // ### ditto
//    }
//// --- END testing ------------


    val ret = translations get apiElemName
    ret getOrElse (throw new Exception("Invalid API element name: " + apiElemName))
  }

  override def toApiElemName(kdvhElemName: String): String = {
    val ret = reverseTranslations get kdvhElemName
    ret getOrElse (throw new Exception("Invalid KDVH element name: " + kdvhElemName))
  }
}

// $COVERAGE-ON$
