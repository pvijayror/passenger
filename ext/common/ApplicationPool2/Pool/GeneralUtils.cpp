/*
 *  Phusion Passenger - https://www.phusionpassenger.com/
 *  Copyright (c) 2011-2015 Phusion
 *
 *  "Phusion Passenger" is a trademark of Hongli Lai & Ninh Bui.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 */

// This file is included inside the Pool class.

protected:

static void runAllActions(const boost::container::vector<Callback> &actions) {
	boost::container::vector<Callback>::const_iterator it, end = actions.end();
	for (it = actions.begin(); it != end; it++) {
		(*it)();
	}
}

static void runAllActionsWithCopy(boost::container::vector<Callback> actions) {
	runAllActions(actions);
}

void verifyInvariants() const {
	// !a || b: logical equivalent of a IMPLIES b.
	#ifndef NDEBUG
	if (!selfchecking) {
		return;
	}
	assert(!( !getWaitlist.empty() ) || ( atFullCapacityUnlocked() ));
	assert(!( !atFullCapacityUnlocked() ) || ( getWaitlist.empty() ));
	#endif
}

void verifyExpensiveInvariants() const {
	#ifndef NDEBUG
	if (!selfchecking) {
		return;
	}
	vector<GetWaiter>::const_iterator it, end = getWaitlist.end();
	for (it = getWaitlist.begin(); it != end; it++) {
		const GetWaiter &waiter = *it;
		const GroupPtr *group;
		assert(!groups.lookup(waiter.options.getAppGroupName(), &group));
	}
	#endif
}

void fullVerifyInvariants() const {
	TRACE_POINT();
	verifyInvariants();
	UPDATE_TRACE_POINT();
	verifyExpensiveInvariants();
	UPDATE_TRACE_POINT();

	GroupMap::ConstIterator g_it(groups);
	while (*g_it != NULL) {
		const GroupPtr &group = g_it.getValue();
		group->verifyInvariants();
		group->verifyExpensiveInvariants();
		g_it.next();
	}
}

bool runHookScripts(const char *name,
	const boost::function<void (HookScriptOptions &)> &setup) const
{
	if (agentsOptions != NULL) {
		string hookName = string("hook_") + name;
		string spec = agentsOptions->get(hookName, false);
		if (!spec.empty()) {
			HookScriptOptions options;
			options.agentsOptions = agentsOptions;
			options.name = name;
			options.spec = spec;
			setup(options);
			return Passenger::runHookScripts(options);
		} else {
			return true;
		}
	} else {
		return true;
	}
}

static const char *maybePluralize(unsigned int count, const char *singular, const char *plural) {
	if (count == 1) {
		return singular;
	} else {
		return plural;
	}
}


public:

Context *getContext() {
	return &context;
}

const SpawningKit::ConfigPtr &getSpawningKitConfig() const {
	return context.getSpawningKitConfig();
}

const UnionStation::CorePtr &getUnionStationCore() const {
	return getSpawningKitConfig()->unionStationCore;
}

const RandomGeneratorPtr &getRandomGenerator() const {
	return getSpawningKitConfig()->randomGenerator;
}
