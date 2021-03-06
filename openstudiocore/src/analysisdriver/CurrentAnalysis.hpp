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

#ifndef ANALYSISDRIVER_CURRENTANALYSIS_HPP
#define ANALYSISDRIVER_CURRENTANALYSIS_HPP

#include <analysisdriver/AnalysisDriverAPI.hpp>

#include <utilities/core/Logger.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>

#include <vector>

namespace openstudio {

namespace runmanager {
  struct JobErrors;
}

namespace analysis {
  class Analysis;
}

namespace analysisdriver {

class AnalysisRunOptions;

namespace detail {

  class CurrentAnalysis_Impl;
  class AnalysisDriver_Impl;

} // detail

/** CurrentAnalysis stores information about an analysis::Analysis being run or ran by
 *  AnalysisDriver. */
class ANALYSISDRIVER_API CurrentAnalysis {
 public:
  /** @name Constructors and Destructors */
  //@{

  CurrentAnalysis(const analysis::Analysis& analysis,
                  const AnalysisRunOptions& runOptions);

  virtual ~CurrentAnalysis() {}

  //@}
  /** @name Getters */
  //@{

  analysis::Analysis analysis() const;

  AnalysisRunOptions runOptions() const;

  //@}
  /** @name Queries */
  //@{

  /** Returns the number of runmanager::Jobs currently being monitored for this analysis by
   *  AnalysisDriver */
  int numQueuedJobs() const;

  /** Returns the number of runmanager::Jobs currently being monitored for this analysis by
   *  AnalysisDriver that originated from a call to Analysis::addDataPoint or
   *  OpenStudioAlgorithm::createNextIteration. */
  int numQueuedOSJobs() const;

  /** Returns the number of runmanager::Jobs currently being monitored for this analysis by
   *  AnalysisDriver that originated from a call to Dakota. */
  int numQueuedDakotaJobs() const;

  /** Returns numJobsInIteration() - numQueuedJobs(). */
  int numCompletedJobsInOSIteration() const;

  /** Returns number of failed jobs. */
  int numFailedJobsInOSIteration() const;

  /** Returns the total number of runmanager::Jobs that need to be queued and simulated to
   *  complete the current OpenStudio iteration. (All data points already present from calling
   *  Analysis::addDataPoint or OpenStudioAlgorithm::createNextIteration.) */
  int totalNumJobsInOSIteration() const;

  /** Returns the JobErrors generated by the DAKOTA job, if applicable and the DAKOTA job has
   *  finished. */
  boost::optional<runmanager::JobErrors> dakotaJobErrors() const;

  /** Returns true if this and other are the exact same object, that is, if they share data. */
  bool operator==(const CurrentAnalysis& other) const;

  /** Negation of operator==. */
  bool operator!=(const CurrentAnalysis& other) const;

  //@}
  /** @name Run Management */
  //@{

  /// Connect signal from this CurrentAnalysis implementation object to slot on qObject.
  bool connect(const std::string& signal,
               const QObject* qObject,
               const std::string& slot,
               Qt::ConnectionType type = Qt::AutoConnection) const;

  void moveToThread(QThread* targetThread);

  //@}
 protected:
  /** Get the impl pointer. */
  boost::shared_ptr<detail::CurrentAnalysis_Impl> getImpl() const;

  /// @cond
  typedef detail::CurrentAnalysis_Impl ImplType;

  friend class detail::CurrentAnalysis_Impl;
  friend class detail::AnalysisDriver_Impl;

  explicit CurrentAnalysis(boost::shared_ptr<detail::CurrentAnalysis_Impl> impl);
  /// @endcond

 private:
  boost::shared_ptr<detail::CurrentAnalysis_Impl> m_impl;

  REGISTER_LOGGER("openstudio.analysisdriver.CurrentAnalysis");
};

/** \relates CurrentAnalysis*/
typedef boost::optional<CurrentAnalysis> OptionalCurrentAnalysis;

/** \relates CurrentAnalysis*/
typedef std::vector<CurrentAnalysis> CurrentAnalysisVector;

} // analysisdriver
} // openstudio

#endif // ANALYSISDRIVER_CURRENTANALYSIS_HPP

