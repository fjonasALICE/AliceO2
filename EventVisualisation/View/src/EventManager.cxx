// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

///
/// \file    EventManager.cxx
/// \author  Jeremi Niedziela
/// \author  Julian Myrcha

#include "EventVisualisationView/EventManager.h"
#include "EventVisualisationView/MultiView.h"
#include "EventVisualisationView/Options.h"
#include "EventVisualisationDataConverter/VisualisationEvent.h"
#include <EventVisualisationBase/DataSourceOnline.h>

#include <TEveManager.h>
#include <TEveTrack.h>
#include <TEveProjectionManager.h>
#include <TEveTrackPropagator.h>
#include <TEnv.h>
#include <TEveElement.h>
#include <TGListTree.h>
#include "FairLogger.h"

#define elemof(e) (unsigned int)(sizeof(e) / sizeof(e[0]))

using namespace std;

namespace o2
{
namespace event_visualisation
{

EventManager* EventManager::instance = nullptr;

EventManager& EventManager::getInstance()
{
  if (instance == nullptr) {
    instance = new EventManager();
  }
  return *instance;
}

EventManager::EventManager() : TEveEventManager("Event", "")
{
  LOG(INFO) << "Initializing TEveManager";
  for (unsigned int i = 0; i < elemof(dataTypeLists); i++) {
    dataTypeLists[i] = nullptr;
  }
}

void EventManager::Open()
{
  setDataSource(new DataSourceOnline(Options::Instance()->dataFolder()));
}

void EventManager::displayCurrentEvent()
{
  if (getDataSource()->getEventCount() > 0) {
    MultiView::getInstance()->destroyAllEvents();
    int no = getDataSource()->getCurrentEvent();

    for (int i = 0; i < EVisualisationDataType::NdataTypes; ++i) {
      dataTypeLists[i] = new TEveElementList(gDataTypeNames[i].c_str());
    }

    auto displayList = getDataSource()->getVisualisationList(no);
    for (auto it = displayList.begin(); it != displayList.end(); ++it) {

      //if (it->second == EVisualisationGroup::TPC) { // temporary
      displayVisualisationEvent(it->first, gVisualisationGroupName[it->second]);
      //}
    }

    for (int i = 0; i < EVisualisationDataType::NdataTypes; ++i) {
      MultiView::getInstance()->registerElement(dataTypeLists[i]);
    }
  }
  MultiView::getInstance()->redraw3D();
}

void EventManager::GotoEvent(Int_t no)
{
  if (getDataSource()->getEventCount() > 0) {
    if (no == -1) {
      no = getDataSource()->getEventCount() - 1;
    }
    this->getDataSource()->setCurrentEvent(no);
    displayCurrentEvent();
  }
}

void EventManager::NextEvent()
{
  if (getDataSource()->getEventCount() > 0) {
    if (this->getDataSource()->getCurrentEvent() < getDataSource()->getEventCount() - 1) {
      Int_t event = (this->getDataSource()->getCurrentEvent() + 1) % getDataSource()->getEventCount();
      GotoEvent(event);
    }
  }
}

void EventManager::PrevEvent()
{
  if (getDataSource()->getEventCount() > 0) {
    if (this->getDataSource()->getCurrentEvent() > 0) {
      GotoEvent(this->getDataSource()->getCurrentEvent() - 1);
    }
  }
}

void EventManager::Close()
{
  delete this->dataSource;
  this->dataSource = nullptr;
}

void EventManager::AfterNewEventLoaded()
{
  TEveEventManager::AfterNewEventLoaded();
}

void EventManager::AddNewEventCommand(const TString& cmd)
{
  TEveEventManager::AddNewEventCommand(cmd);
}

void EventManager::RemoveNewEventCommand(const TString& cmd)
{
  TEveEventManager::RemoveNewEventCommand(cmd);
}

void EventManager::ClearNewEventCommands()
{
  TEveEventManager::ClearNewEventCommands();
}

EventManager::~EventManager()
{
  instance = nullptr;
}

void EventManager::DropEvent()
{
  DestroyElements();
}

void EventManager::displayVisualisationEvent(VisualisationEvent& event, const std::string& detectorName)
{
  size_t trackCount = event.getTrackCount();
  LOG(INFO) << "displayVisualisationEvent: " << trackCount << " detector: " << detectorName;
  // tracks
  auto* list = new TEveTrackList(detectorName.c_str());
  list->IncDenyDestroy();
  // clusters
  size_t clusterCount = 0;
  auto* point_list = new TEvePointSet(detectorName.c_str());
  point_list->IncDenyDestroy();
  point_list->SetMarkerColor(kBlue);

  for (size_t i = 0; i < trackCount; ++i) {
    VisualisationTrack track = event.getTrack(i);
    TEveRecTrackD t;
    //double* p = track.getMomentum();
    //t.fP = {p[0], p[1], p[2]};
    t.fSign = track.getCharge() > 0 ? 1 : -1;
    auto* vistrack = new TEveTrack(&t, &TEveTrackPropagator::fgDefault);
    vistrack->SetLineColor(kMagenta);
    size_t pointCount = track.getPointCount();
    vistrack->Reset(pointCount);

    for (size_t j = 0; j < pointCount; ++j) {
      auto point = track.getPoint(j);
      vistrack->SetNextPoint(point[0], point[1], point[2]);
    }
    list->AddElement(vistrack);

    // clusters connected with track
    for (size_t i = 0; i < track.getClusterCount(); ++i) {
      VisualisationCluster cluster = track.getCluster(i);
      point_list->SetNextPoint(cluster.X(), cluster.Y(), cluster.Z());
      clusterCount++;
    }
  }

  if (trackCount != 0) {
    dataTypeLists[EVisualisationDataType::Tracks]->AddElement(list);
  }

  // global clusters (with no connection information)
  for (size_t i = 0; i < event.getClusterCount(); ++i) {
    VisualisationCluster cluster = event.getCluster(i);
    point_list->SetNextPoint(cluster.X(), cluster.Y(), cluster.Z());
    clusterCount++;
  }

  if (clusterCount != 0) {
    dataTypeLists[EVisualisationDataType::Clusters]->AddElement(point_list);
  }
  LOG(INFO) << "tracks: " << trackCount << " detector: " << detectorName << ":" << dataTypeLists[EVisualisationDataType::Tracks]->NumChildren();
  LOG(INFO) << "clusters: " << clusterCount << " detector: " << detectorName << ":" << dataTypeLists[EVisualisationDataType::Clusters]->NumChildren();
}

} // namespace event_visualisation
} // namespace o2
