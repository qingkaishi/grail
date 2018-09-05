/*
 * This file implements two containers, PushPopVector
 * and PushPopCache, which support push/pop operations.
 *
 * Whenever a pop operation is called, the elements
 * added to the structure after last push operation.
 *
 * PushPopCache guarantees the uniqueness of the elements
 * in the container, while PushPopVector does not have the
 * guarantee.
 *
 * Added on 26/10/2015 by Qingkai
 */

#ifndef UTILS_PUSHPOPCACHE_H
#define UTILS_PUSHPOPCACHE_H

#include <vector>
#include <assert.h>

class CFLStack {
protected:
    typedef struct Item {
      int PrevIndex;
      int NextIndex;
      int SelfIndex;
      int Label;

      Item(int p, int n, int s, int l) : PrevIndex(p), NextIndex(n), SelfIndex(s), Label(l) {}
      ~Item() {}
    } Item;

	std::vector<size_t> SizeStack;
	std::vector<Item> LabelVector;
	size_t CallingDepth;

public:
	CFLStack() {
	    // push a base
	    LabelVector.emplace_back(-1, 1, 0, 0);
	    LabelVector.emplace_back(0, 2, 1, 0);
	    CallingDepth = 0;
	}

	virtual ~CFLStack() {
	}

	virtual bool add(int N) {
	    assert(N != 0 && "Zero is not regarded as a label!");
	    if (N > 0) {
	        LabelVector.back().Label = N;

	        int CurSize = LabelVector.size();
	        LabelVector.emplace_back(CurSize - 1, CurSize + 1, CurSize, 0);

	        CallingDepth++;
	    } else {
	        // N < 0
	        if (LabelVector[LabelVector.back().PrevIndex].Label <= 0) {
	            // no need to match
	            LabelVector.back().Label = N;

	            int CurSize = LabelVector.size();
	            LabelVector.emplace_back(CurSize - 1, CurSize + 1, CurSize, 0);

	            CallingDepth++;
	        } else {
	            // need to match
	            Item& Label2MatchItem = LabelVector[LabelVector.back().PrevIndex];
	            if (Label2MatchItem.Label + N == 0) {
	                // can match
	                LabelVector.back().Label = N;

	                int CurSize = LabelVector.size();
	                LabelVector.emplace_back(CurSize - 1, CurSize + 1, CurSize, 0);

	                LabelVector.back().PrevIndex = LabelVector[Label2MatchItem.PrevIndex].SelfIndex;
	                LabelVector[Label2MatchItem.PrevIndex].NextIndex = CurSize;

	                CallingDepth--;
	            } else {
	                // cannot match
	                return false;
	            }
	        }

	    }
	    return true;
	}

	virtual void push() {
		SizeStack.push_back(LabelVector.size());
	}

	virtual void pop() {
		assert(!SizeStack.empty() && "The push and pop operations are mis-matched!");
		size_t TargetSz = SizeStack.back();
		SizeStack.pop_back();

		if (TargetSz == LabelVector.size())
		    return;

		assert(TargetSz < LabelVector.size());
		while (TargetSz < LabelVector.size()) {
		    auto PopedItem = LabelVector.back();
		    LabelVector.pop_back();

		    // reset the top
		    auto& CurrTopItem = LabelVector.back();
		    CurrTopItem.Label = 0;

		    if (LabelVector[PopedItem.PrevIndex].SelfIndex == LabelVector.size() - 1) {
		        CallingDepth--;
		    } else {
		        CallingDepth++;
		        LabelVector[PopedItem.PrevIndex].NextIndex = LabelVector[CurrTopItem.PrevIndex].SelfIndex;
		    }
		}
	}

	virtual void reset() {
		SizeStack.clear();
		LabelVector.clear();

        LabelVector.emplace_back(-1, 1, 0, 0);
        LabelVector.emplace_back(0, 2, 1, 0);
        CallingDepth = 0;
	}

	bool empty() const {
		return LabelVector.size() == 2 && LabelVector[1].Label == 0;
	}

	size_t callingDepth() const {
	    return CallingDepth;
	}
};


#endif
