#pragma once

#include <string>
#include <map>
#include <vector>
#include <unordered_map>
#include <list>
#include <typeinfo>
#include <set>

#include "DCS/Lua/lua.hpp"

#pragma comment(lib, "DCS/libs/CockpitBase.lib")
#pragma comment(lib, "DCS/libs/edCore.lib")

class RandomObject;
class MovingObject;

namespace osg {
	class Vec3f;
}
namespace Graphics {
	class ModelParser;
	class effectState;
}

namespace Mail {
	class Stream;
}

namespace cockpit {
	class clickableElementActionData;
	class clickableElementData;
	class UpdateHandler;
	class SimpleOctTree;
}


class LinkBaseItem {
public:
	LinkBaseItem* prev;
	LinkBaseItem* next;

	LinkBaseItem() { next = prev = this; }
	void Clear() { next = prev = this; }
	LinkBaseItem& operator=(LinkBaseItem const&) { return *this; }

	//size 0x10
};

class LinkHost {
	friend class LinkBase;
public:
	void ResetLinks();
protected:
	LinkBaseItem baseItem;

	//size 0x10
};


class LinkBase : public LinkBaseItem {
	friend class LinkHost;
public:
	LinkBase(LinkBase const&);
	LinkBase();
	LinkBase& operator=(LinkBase const&);
protected:
	void Set(LinkHost*);
public:
	~LinkBase();
	LinkHost* Get() const { return host; }
protected:
	LinkHost* host;

	//size 0x18
};

template <class T = LinkHost> class Link : public LinkBase
{
public:
	Link(void) {}
	Link(const Link<T>& source) { Set(source); }
	Link(T* h) { Set(h); }

	operator T* (void) const { return static_cast<T*>(host); }
	T* operator ->(void) const { return static_cast<T*>(host); }
	Link& operator =(T* h) { Set(h); return *this; }

	T& operator [](int i) const { return static_cast<T*>(host)[i]; }

	//size 0x18
};

namespace Sound {
	struct SND_SourceParams {
		int fields;
		double position[3];
		double orientation[4];
		float  gain;
		float  pitch;
		float  radius;
		float  lowpass;
	};

	class Host {
	public:
		int id;
	};

	class Source {
	public:
		void create(Host&, char const*, struct SND_SourceParams const*);
		void create_alternative(Host&, char const*, char const*, SND_SourceParams const*);
		int create_alternative_ex(Host&, char const*, char const*, SND_SourceParams const*);
		void destroy();
		bool is_playing() const;
		bool link(Source&);
		void play(struct SND_PlayParams const*, SND_SourceParams const*);
		bool play_continue();
		void play_once(SND_SourceParams const*);
		void play_update(SND_SourceParams const*);
		void stop();
		bool unlink();
		void update(struct SND_SourceParams const*);

	protected:
		int source;
		//size 0x08
	};


}

namespace ed {
	template <typename T> class basic_string : public std::basic_string<T> {
	public:
		basic_string() = default;
		basic_string(basic_string const&) = default;
		basic_string(basic_string&&) = default;
		basic_string(const std::basic_string<T>& s) : std::basic_string<T>(s) {}
		basic_string& operator=(basic_string const&) = default;
		basic_string(const char* string) : std::basic_string<T>(string) {};

		basic_string<T>& appendf(T const*, ...);
		basic_string& format(char const*, ...);

	};
	typedef basic_string<char> string;

	template <typename T> class allocator : public std::allocator<T> { };
	template <typename T, typename A = allocator<T>> class vector : public std::vector<T, A> {};
	template<typename Key, typename T, typename Hash = std::hash<Key>, typename Pred = std::equal_to<Key>, typename Alloc = allocator<std::pair<const Key, T> > > class unordered_map : public std::unordered_map<Key, T, Hash, Pred, Alloc> {};
	template <typename T, std::size_t S> class array : public std::array<T, S> {};
	template <typename T, typename A = allocator<T>> class list : public std::list<T, A> {};
	template <typename T> class set : public std::set<T> {};
	template<typename Key, typename T, typename Compare = std::less<Key>, typename Alloc = allocator<std::pair<const Key, T> > > class map : public std::map<Key, T, Compare, Alloc> {};


}


namespace Common {
	template <typename T> class FakeUnknown : public T {
	private:
		virtual void FinalRelease() {};
	};

	class Identifier {
	public:
		Identifier();
		Identifier(const char* param_1);
		Identifier(ed::basic_string<char>* param_1);
		void* id;
		//size 0x08
	};

	class ISharedObject {
	private:
		virtual void FinalRelease() = 0;
	public:
		virtual ~ISharedObject() {};
		void AddReference() { shared_count++; }
		void Release() { if (--shared_count == 0) FinalRelease(); }
	protected:
		int shared_count = 0;
		//size 0x10
	};

	class Identifiable : public ISharedObject {
	private:
	protected:
	public:
		Common::Identifier identifier;
		//size 0x18
	};

	template <typename T> class Unknown : public T {
	private:
		virtual void FinalRelease() { delete this; };
	};

	class Factory : public Unknown<ISharedObject> {
	public:
		virtual ~Factory();
		virtual Identifiable* createInstance(Identifier const&) = 0;
		virtual void destroyInstance(Identifiable* id) {
			delete id;
		};
		void addIdentifier(const Identifier& identifier);
		void removeIdentifier(const Identifier& identifier);
		void createInstance(const Identifier& identifier, Identifiable** identifiable){
			*identifiable = createInstance(identifier);
			(*identifiable)->AddReference();
		}
	protected:
		std::list<Identifier> identifierList;
		//size 0x20 
	};

	class FactoryManager : public Factory {
	public:
		virtual ~FactoryManager();
		virtual Common::Identifiable* createInstance(Identifier const&);
		virtual void destroyInstance(Common::Identifiable*);
		void removeFactory(Factory* factory);
	protected:
		void addFactory(Factory* factory);

	public:
		void add(Factory* factory) { addFactory(factory); }
		struct FactoryMap {
			Identifier identifier;
			Factory* factory;
		};
		ed::vector<FactoryMap> factory_map;
		ed::vector<Factory*> factories;

		//size 0x50
	};

	template <class T> inline Identifier identify(T* t = 0) {
		return Identifier(typeid(T).name());
	}

	template <typename T> class StandardFactory : public virtual Factory {
	public:
		virtual Identifiable* createInstance(Common::Identifier const& ident) {
			//if (!check_license()) return nullptr;
			Identifiable* instance = new T;
			instance->identifier = ident;
			return instance;
		}

		StandardFactory() {
			addIdentifier(identify<T>());
		}
	};

}

void getRegistry(Common::FactoryManager** registry);

template <typename T> class WorldFactory : public Common::StandardFactory<T> {
protected:
	inline static WorldFactory* registered_ptr = 0;
public:
	WorldFactory() {
		if (registered_ptr == 0) {
			Common::FactoryManager* fm;
			getRegistry(&fm);
			if (fm) {
				fm->add(this);
				registered_ptr = this;
				atexit(ExitUnregister);
			}
		}
	}
	void unregister() {
	}
protected:
	static void ExitUnregister() {
		if (registered_ptr != 0) {
			Common::FactoryManager* fm;
			getRegistry(&fm);
			fm->removeFactory(registered_ptr);
			registered_ptr = 0;
		}
	}
};


template <class T> class HeapVector {
public:
	unsigned int size() const {
		return buffersize;
	}

	T& operator [](int index) {
		if ((index < 0) || (index >= (int)buffersize)) {
			return T();
		}
		return data[index];
	}

	const T& operator [](int index) const {
		if ((index < 0) || (index >= (int)buffersize)) {
			return T();
		}
		return data[index];
	}

protected:
	T* data;
	unsigned int buffersize;
	unsigned int reserved;
	int type;
};

namespace Graphics {
	struct DynamicParam {
		union
		{
			void* p;
			float f;
			int i;
		};
		DynamicParam(float _f) {
			f = _f;
		}
		DynamicParam() {
			p = 0;
		}
	};

	typedef HeapVector<DynamicParam> DParamList;
}

namespace Lua {
	class Loader;

	class Config {
	public:
		Config(lua_State* lua, int i = LUA_GLOBALSINDEX) : L(lua), top(lua_gettop(lua)) { lua_pushvalue(L, i); }
		~Config() { lua_settop(L, top); }
		bool open(int);
		bool open(char const*);
		void get(int);
		void get(char const*);
		template <typename T> bool get(char const* path, T* v) { get(path); return pop(v); };
		template <typename T> bool get(const char* path, T* v, const T& d) { get(path); if (pop(v)) return true; else { *v = d; return false; } }
		template<typename T> T     get(int key, const T& v) { T r(v); get(key); pop(&r); return r; }
		template<typename T> T     get(const char* path, const T& v) { T r(v); get(path); pop(&r); return r; }
		template<typename T>  bool get(int key, T* v, const T& d) { get(key); return pop(v) ? true : (*v = d), false; }
		template<typename T>  bool get(int key, T* v) { get(key); return pop(v); }
		template<typename T> T* getDevice() {
			get("link");
			void* addr = 0;
			pop(&addr);
			return (T*)(addr);
		}
		void pop(int n = 1) { lua_pop(L, n); }
		bool pop(unsigned char*);
		bool pop(unsigned short*);
		bool pop(int*);
		bool pop(unsigned int*);
		bool pop(float*);
		bool pop(double*);
		bool pop(void**);
		bool pop(ed::basic_string<char>*);
		bool pop(unsigned long long*);
		bool pop(bool*);


	protected:
		lua_State* L;
		int top;
	};
}


template <class T> class wPosition3{
public:
	struct vec { T x; T y; T z; };
	vec x; T x_p;
	vec y; T y_p;
	vec z; T z_p;
	vec p; T p_p;
};

namespace cockpit {


	class ccContextRelatedObject : public Common::FakeUnknown<Common::Identifiable> {
	public:
		virtual ~ccContextRelatedObject();
		ccContextRelatedObject(ccContextRelatedObject const&);
		ccContextRelatedObject();
		class ccCockpitContext* get_context()const;

		ccContextRelatedObject& operator=(ccContextRelatedObject&&);
	protected:
		class ccCockpitContext* cccockpitcontext;

		//size 0x20
	};

	class avLuaRegistrable {
	public:
		avLuaRegistrable(avLuaRegistrable&&);
		avLuaRegistrable(avLuaRegistrable const&);
		avLuaRegistrable();
	protected:
		virtual void l_register_functions(lua_State*, int) = 0;
		template<class T> static inline T* l_get_device(lua_State* L) {
			if (!L) return nullptr;
			T* device(nullptr);

			if (lua_istable(L, 1)) {
				lua_getfield(L, 1, "link");
				if (lua_isuserdata(L, -1)) {
					device = static_cast<T*>((avLuaRegistrable*)lua_touserdata(L, -1));
				}
				lua_pop(L, 1);
			}

			if (!device) {
				lua_getglobal(L, "link");
				if (lua_isuserdata(L, -1)) {
					device = static_cast<T*>((avLuaRegistrable*)lua_touserdata(L, -1));
				}
				lua_pop(L, 1);
			}
			return device;
		}

		//size 0x08
	};

	struct animation_element_data{
		void  read(Lua::Config& config);

		int arg = 0;
		int clickable_command = 0;
		float arg_increment = 0.0f;
		float arg_lim_min = 0.0f;
		float arg_lim_max = 0.0f;
		bool  direct_set = false;
	};


	class avDevice :public ccContextRelatedObject, public avLuaRegistrable {
	public:
		virtual ~avDevice();
		virtual void initialize(unsigned char id, ed::basic_string<char> const& name, ed::basic_string<char> const& luaname);
		virtual void release();
		virtual void SetCommand(int, float);
		virtual void OnCockpitEvent(ed::basic_string<char> const&, HeapVector<Graphics::DynamicParam> const&);
		virtual void update();
		virtual void post_initialize();
		virtual bool checkCallbackKeyboard(int, float&);
		virtual bool checkCallbackClickable(int, float&);
		virtual bool checkAnimationConditions(int, int, float&);
		virtual void external_parse(Graphics::ModelParser&, wPosition3<float> const&, Graphics::effectState*);
		virtual bool NetCrewMemberUpdateStream(unsigned char idx, unsigned char client, Mail::Stream& stream);
	protected:
		virtual void register_in_script(struct lua_State*);
		virtual char const* l_metatable_name()const;
		virtual void l_register_functions(struct lua_State*, int idx);

	public:
		clickableElementData* getClickableElement(int);
		UpdateHandler* getUpdateHandler();
		void hide_callback_element(int, bool);
		unsigned char ID()const;
		void insertCallbackElement(int, clickableElementData*);
		bool is_active()const;
		void lua_call_SetCommand(int, float);
		ed::basic_string<char> const& Name()const;
		avDevice& operator=(avDevice const&);
		void performClickableAction(int command, float value, bool ignoreSetCommand = false);
		void setUpdateHandler(class UpdateHandler*);
		void start(unsigned int);

		lua_State* getLua() { return L; }

		__declspec(dllimport) static ed::basic_string<char> const common_path;
		__declspec(dllimport) static RandomObject device_random_evenly;
		__declspec(dllimport) static RandomObject device_random_orderly;
		__declspec(dllimport) static bool EasyFlight;
		__declspec(dllimport) static bool EasyRadar;

	protected:
		void accelerateValue(float&, struct std::pair<double, double>&);
		void axis_value_iterate(int, float);
		void button_value_iterate(int, bool);
		void button_value_ON_OFF(int, bool);
		void close_LuaState();
		lua_State* ensured_Lua(char const* name = nullptr);
		void make_default_activity(double);
		void switcher_value_iterate(int, int);
		void switcher_value_iterate_reversible(int, int*);

		bool initialized;
		unsigned char device_id;
		ed::basic_string<char> name;
		lua_State* L;
		class avDevice_BasicTimer* timer;
		std::map<int, clickableElementData*> clickable_elements;
		std::map<int, ed::vector<animation_element_data>> animations;
		bool init_script_from_mission;
		UpdateHandler* update_handler_ptr;
		bool has_script_file;
		bool active;
		int set_command;

		//size 0x98
	};

	class avBreakable {
	public:
		virtual void setflag_failure(bool);
		virtual void setflag_ready(bool);
		virtual bool getflag_failure()const;
		virtual bool getflag_ready()const;
		virtual void repair();
	protected:
		virtual void check_damage(unsigned int, double, struct avDamageData&);
		virtual bool set_damage(unsigned int, bool);
	public:
		virtual bool set_failure(ed::basic_string<char> const&);

	public:
		avBreakable(avBreakable const&);
		avBreakable();
		~avBreakable();
		avBreakable& operator=(avBreakable const&);

		//static ed::list<avBreakable*, ed::allocator<avBreakable*>> const& get_damage_capable_list();
		void on_planned_failure(ed::basic_string<char> const&, double, double, double);
		void process_damage(unsigned int, double);

	protected:
		void load_from_state(lua_State*);
		void register_as_breakable();
		bool set_failure(unsigned int);
		void unregister_as_breakable();

	private:
		static void createHumanFailureEvent(ed::basic_string<char> const&, ed::basic_string<char> const&);
		static void release_work_time_failures();
		static void start_work_time_failures();
		static ed::list<avBreakable*> const& get_damage_capable_list();

		void on_timer_event(unsigned int, ed::basic_string<char> const&, ed::basic_string<char> const&);
		void add_failure_timer(unsigned int const&, double const&, ed::basic_string<char> const&, ed::basic_string<char> const&);

		__declspec(dllimport) static class RandomObject random_evenly;
		bool broken;
		bool flag_ready;
		bool registered;

		ed::list<avBreakable*> failure_list;
		//size 0x20;
	};


	class avLuaDevice : public avDevice, public avBreakable {
	public:
		virtual ~avLuaDevice();
		virtual void initialize(unsigned char id, ed::basic_string<char> const& name, ed::basic_string<char> const& script);
		virtual void release();
		virtual void SetCommand(int cmd, float value);
		virtual void OnCockpitEvent(ed::basic_string<char> const&, HeapVector<struct Graphics::DynamicParam> const&);
		virtual void update();
		virtual bool checkAnimationConditions(int, int, float&);
	private:
		virtual void register_in_script(struct lua_State*);
	public:
		virtual bool set_damage(unsigned int, bool);

		avLuaDevice(avLuaDevice const&);
		avLuaDevice();
		avLuaDevice& operator=(avLuaDevice const&);

	private:
		static int l_set_damage(struct lua_State*);
		static int l_make_default_activity(struct lua_State*);
		void set_timer(double);
	protected:
		void l_register_functions(struct lua_State* L, int i);
		int has_update_function;
		int has_check_animation_conditions_function;


		//extra
	public:
		inline virtual void p_register_in_script(lua_State* L) { avLuaDevice::register_in_script(L); }

		//size 0xc0
	};

	class avBasicElectric {
	public:
		virtual void setElecPower(bool);
		virtual void switchElecOnOff();
		virtual bool getElecPower() const;
		avBasicElectric();
		avBasicElectric(avBasicElectric&&);
		avBasicElectric& operator=(avBasicElectric&&);
	protected:
		bool elecPower;
		//size 0x10
	};

	class RWR_Emitter {

	};

	class avRWR : public avLuaDevice, public avBasicElectric {
	public:
		//ccContextRelatedObject
		avRWR(void);
		virtual ~avRWR();
		virtual void initialize(unsigned char, ed::basic_string<char> const&, ed::basic_string<char> const&);
		virtual void release(void);
		virtual void update(void);
	protected:
		virtual void checkLaunchEvent(void);

		void changeBrightness(void);
		void ClearEmitters(void);
		void dismissTgtSeparartion(void);
		float getBrightness(void)const;
		class ed::vector<RWR_Emitter> const& GetEmitters(void);
		bool getLaunchEventIsActive(void)const;
		MovingObject* getLaunchingSource(void)const;
		bool getLockEventIsActive(void)const;
		MovingObject* getLockingSource(void)const;
		unsigned int getLockingType(void)const;
		int GetMainEmitterPlace(void)const;
		unsigned int GetNewestEmitter(void)const;
		bool getPriorityMode(void)const;
		void initialize_storage(void);
		bool const isDegraded(void)const;
		void search(int, int);
		void setBrightness(float);
		void setLaunchEventIsActive(bool);

	protected:
		float calc_priority(unsigned int, unsigned int, float)const;
		void calculateGroups(void);
		bool checkEmitter(unsigned int, unsigned int);
		void checkEmittersSize(void);
		void checkLockEvent(void);
		int findNewPlace(void);
		int findWithLowestPriority(float&);
		bool getEmitterPos(MovingObject*, wPosition3<float>&, float&, float&, class osg::Vec3f&);
		int getEmittersSize(void)const;
		float getSignalStreigth(MovingObject*, wPosition3<float> const&, int, int, float)const;
		bool isPositionEmpty(wPosition3<double> const&, int);
		void ResizeStorage(int);
		void selectMainEmitter(void);
		void separateGroups(void);
		void SetEmitterIsNotValid(int);
		void SetEmitterIsValid(int);
		void setEmitterTypes(MovingObject*, RWR_Emitter&);
		void updateEmitters(void);

	protected:
		ed::vector<void*> sensor_vector;
		ed::vector<RWR_Emitter*> rwr_emitters;
		bool some_bool;
		bool launchEventIsActive;
		bool lockEventIsActive;
		bool priorityMode;
		bool update_separate_groups_related_1;
		bool update_separate_groups_related_2;
		SimpleOctTree* octree;

		ed::set<int> groups[8];
		char cross[16][16];
		ed::set<int> search_set;

		float brightness;
		int emittersSize;
		unsigned int newestEmitter;
		RWR_Emitter* mainEmitter;
		int mainEmitterPlace;
		unsigned short MaxThreats;
		double EmitterLiveTime;
		Link<MovingObject> lauchingSource;
		Link<MovingObject> lockingSource;
		int lockingType;

		Sound::SND_SourceParams SndParams;
		void* some_data_1;
		void* some_data_2;
		void* some_data_3;

		struct {
			ed::map<unsigned int, Sound::Source*> search_new;
			ed::map<unsigned int, Sound::Source*> lock_new;
			Sound::Source  launch_warning;
			Sound::Source  threat_new;
		} sound;

		double RWR_detection_coeff;

		//size 0x3a0
	};


}
