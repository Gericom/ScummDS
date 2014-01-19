#ifndef __VERBS_H__
#define __VERBS_H__

enum ClickArea {
	kVerbClickArea = 1,
	kSceneClickArea = 2,
	kInventoryClickArea = 3,
	kKeyClickArea = 4,
	kSentenceClickArea = 5
}; 

void checkExecVerbs();

#endif