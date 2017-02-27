#pragma once
#ifndef PROFILER_H
#define PROFILER_H

#include "global.h"
#include "profile/profile_data.h"
#include <unordered_map>

class Profiler {
public:
	typedef int RootId;
	typedef int NodeId;
	typedef std::unordered_map<NodeId, ProfileData> Collection;
	DISABLE_COPY_AND_ASSIGN(Profiler);

	static Profiler &Instance() {
		static Profiler instance;
		return instance;
	}
	const std::unordered_map<RootId, Collection> *collections() const { return &collections_; }
	bool enable() const { return enable_; }
	void SetEnable(bool value) { enable_ = value; }
	void Start(RootId root_id) { if (enable_) current_collection_ = &collections_[root_id]; }
	void End() { current_collection_ = NULL; }
	void Reset() { collections_.clear(); current_collection_ = NULL; }
	void AddProfileData(NodeId node_id, clock_t consumed_clocks) {
		if (!current_collection_) return;
		ProfileData &data = (*current_collection_)[node_id];
		++data.times;
		data.clocks += consumed_clocks;
	}

private:
	Profiler() : current_collection_(NULL), enable_(false) {}

private:
	std::unordered_map<RootId, Collection> collections_;
	Collection *current_collection_;
	bool enable_;
};

#endif // !PROFILER_H
