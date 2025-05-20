#include <efsw/efsw.hpp>
#include <iostream>

int main( int, char** ) {
	struct UpdateListener : efsw::FileWatchListener {
		virtual void handleFileAction(efsw::WatchID, const std::string&, const std::string&, efsw::Action, std::string) override {}
	};

	for (int i = 0; i <= 10000; ++i) {
		if (i < 20 || i % 20 == 0)
			std::cout << "i: " << i << std::endl;

		efsw::FileWatcher watcher;
		watcher.watch();
		{
			UpdateListener listener;
			auto id = watcher.addWatch(".", &listener);
			watcher.removeWatch(id);
		}
	}

	return 0;
}
