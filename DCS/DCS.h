#pragma once

#include <string>
#include <map>
#include <vector>
#include <unordered_map>
#include <list>
#include <typeinfo>
#include <set>
#include <functional>

#include "DCS/Lua/lua.hpp"

#pragma comment(lib, "DCS/libs/CockpitBase.lib")
#pragma comment(lib, "DCS/libs/edCore.lib")

class RandomObject;
class MovingObject;

class wsType {
public:
	union {
		struct {
			unsigned char    l1;
			unsigned char    l2;
			unsigned char    l3;
			unsigned char    l4;
		};
		long long type;
	};
};

namespace osg {
}

namespace Graphics {
	class ModelParser;
	class effectState;
	class Material;
	class Model;
	class RenderObject;
}

namespace Mail {
	class Stream;
}

namespace cockpit {
	class clickableElementActionData;
	class clickableElementData;
	class UpdateHandler;
	class SimpleOctTree;
	class Element;
}

namespace gui {
	class Window;
}

enum TVScreenCondition {
	Black = 0,
	TV = 1,
	Clouds = 2,
	//IR = 2,
	AG_Radar = 3,
	TerrainLevel = 4,
	Terrain = 5,
	TerrainOther = 6,
	IRInverted = 7,
	Still = 8,
	Chart = 9,
	TexturedTerrain,
	IR,
};

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


namespace osg {
	class Vec3f {
	public:
		float x, y, z;
	};

	class Vec3d {
	public:
		double x, y, z;
	};
}

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

	struct DefaultDeleter {
		void operator()(void* ptr) {
			delete ptr;
		}
	};

	template<class T, class D = DefaultDeleter>
	class Ptr {
	private:
		T* ptr;
		T* ptr2;
	};

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



	typedef ed::Ptr<Material> MaterialPtr;
	typedef ed::Ptr<Model> ModelPtr;

	class Camera {
		char vars[0x310];
	};


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

namespace symb {
	class ShaderLineParams {
	public:
		double thickness;
		double fuzziness;
		bool drawAsWire;
		bool useSpecularPass;
	};
}

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
	public: 
		RWR_Emitter& operator=(RWR_Emitter&&);
		RWR_Emitter& operator=(RWR_Emitter const&);
		void reset(void);
		RWR_Emitter(RWR_Emitter&&);
		RWR_Emitter(RWR_Emitter const&);
		RWR_Emitter(void);
		~RWR_Emitter(void);
		float Priority;
		float SignalStreight;
		osg::Vec3f Direction;
		float Azimuth;
		float Elevation;
		double BirthTime;
		double MissileTime;
		unsigned int id;
		Link<MovingObject> link;
		int RadarType;
		int RadarMode;
		wsType EmitterType;
		void* some_data;
		ed::string EmitterTypeStr;
		int SignalType;
		int PlatformType;
		bool NewEmitter;
		bool Spike;
		int WarningCounter;
		bool DoNotControlEmissionTime;
		bool IsActiveMissile;
		bool IsValid;
		bool isLocked;
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


	struct indicator_viewport {
		double x = 0.0;
		double y = 0.0;
		double w = 100.0;
		double h = 100.0;
	};

	struct indicator_geometry {
		ed::vector<ed::basic_string<char>> shape_names;
		osg::Vec3f pos;
		osg::Vec3f rot;
		osg::Vec3f pos_l;
		osg::Vec3f rot_l;
		int render_target = -1;
		float size_x = 0.0f;
		float size_y = 0.0f;
	};

	
	class ccDrawable : public ccContextRelatedObject, public avLuaRegistrable {
	public:
		enum DrawLevel : char
		{
			DrawLevelWorld = 0,
			DrawLevelScreen = 1,
			DrawLevelTexture = 2,
		};

		virtual ~ccDrawable();
		virtual void initialize(avDevice*, unsigned char, ed::basic_string<char> const&);
		virtual void post_initialize(); //_guard_check_icall
		virtual void release();
		virtual void draw() = 0;
		virtual void draw(unsigned char level) = 0;
		virtual void draw_screenspace(bool) = 0;
		virtual void draw_to_viewport(indicator_viewport const&) = 0;
		virtual void draw_to_render_target();
		virtual bool is_draw_screenspace() const = 0;
		virtual void update() = 0;
		virtual void update(unsigned char level) = 0;
		virtual void control() = 0;
		virtual void start(unsigned int);
		virtual void SetCommand(int, float);
		virtual void OnCockpitEvent(ed::basic_string<char> const&, HeapVector<Graphics::DynamicParam> const&); //_guard_check_icall
		virtual void push_purpose(int) = 0;
		virtual void pop_purpose() = 0;
		virtual void remove_purpose(int) = 0;
		virtual void set_geometry(indicator_geometry const&) = 0;
		virtual Graphics::Camera const& get_camera() = 0;
		virtual osg::Vec3d get_camera_point() = 0;
		virtual TVScreenCondition get_screen_condition() = 0;
		virtual void get_screen_params(HeapVector<Graphics::DynamicParam>&) = 0; //_guard_check_icall = 0
		virtual bool get_render_target_always() const = 0;
		virtual void draw_temporal_set(Element*, unsigned int set, unsigned char level) = 0; //_guard_check_icall = 0
		virtual int getType() const = 0;
		virtual std::unordered_map<ed::basic_string<char>, void (*)(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&), std::hash<ed::basic_string<char>>, std::equal_to<ed::basic_string<char>>, ed::allocator<std::pair<ed::basic_string<char> const, void(*)(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&)>>> const& get_elements_controllers() const = 0;
		virtual std::unordered_map<ed::basic_string<char>, void (*)(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&), std::hash<ed::basic_string<char>>, std::equal_to<ed::basic_string<char>>, ed::allocator<std::pair<ed::basic_string<char> const, void(*)(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&)>>>& get_elements_controllers_ref() = 0;
		virtual std::unordered_map < ed::basic_string<char>, std::function<void(Element*, HeapVector<Graphics::DynamicParam>&)>, std::hash<ed::basic_string<char>>, std::equal_to<ed::basic_string<char>>, ed::allocator<std::pair<ed::basic_string<char> const, std::function<void(Element*, HeapVector<Graphics::DynamicParam>&)>>>> const& get_elements_controllers_lambdas() const = 0;
		virtual std::unordered_map < ed::basic_string<char>, std::function<void(Element*, HeapVector<Graphics::DynamicParam>&)>, std::hash<ed::basic_string<char>>, std::equal_to<ed::basic_string<char>>, ed::allocator<std::pair<ed::basic_string<char> const, std::function<void(Element*, HeapVector<Graphics::DynamicParam>&)>>>>& get_elements_controllers_lambdas_ref() = 0;
		virtual int const get_render_target() const = 0;
		virtual Element* get_element_on_current_page(char const*) = 0;
		virtual wPosition3<float> const& get_position(bool) const = 0;
		virtual wPosition3<float> const& get_initpos() const = 0;
	protected:
		virtual void RegisterInScript(lua_State*); //_guard_check_icall
		virtual void l_register_functions(struct lua_State*, int) = 0;

	public:
		ccDrawable(ccDrawable const&);
		ccDrawable();
		static void clearStaticResources();
		static Graphics::MaterialPtr find_material(ed::basic_string<char> const&);
		static std::unordered_map<ed::basic_string<char>, ed::Ptr<class ceFont>> fonts_cockpit;
		avDevice* get_controller();
		static float get_random();
		symb::ShaderLineParams const& getShaderLineVariableParams() const;
		static void initMaterials(Lua::Config&);
		void make_default_activity(double);
		static std::unordered_map<ed::basic_string<char>, Graphics::MaterialPtr, std::hash<ed::basic_string<char>>, std::equal_to<ed::basic_string<char>>> materials;
	protected:
		void new_controller(char const*, std::function<void(Element*, HeapVector<Graphics::DynamicParam>&)> const&);
	public:
		ccDrawable& operator=(ccDrawable const&);
	protected:
		__declspec(dllimport) static RandomObject randomEvenly;
	public:
		__declspec(dllimport) static ed::vector<ccDrawable*> render_targets;
		void set_controller(avDevice*);

	protected:
		void release_render_target_source(int);

	protected:
		lua_State* L;
		class ccDrawable_BasicTimer* timer;
		avDevice* controller;
		bool test_drawable;
		symb::ShaderLineParams shaderLineParams;

		//size 0x60 2.9
	};

	struct Device_Mode {
		void clear_sub_levels();
		Device_Mode(Device_Mode const&);
		Device_Mode(unsigned char, unsigned char, unsigned char, unsigned char);
		void load_from_state(Lua::Config&);
		bool operator!=(Device_Mode const&) const;
		bool operator==(Device_Mode const&) const;
		unsigned char& operator[](int);
		unsigned char operator[](int) const;
		void save_to_state(lua_State*) const;

		unsigned char master;
		unsigned char level_2;
		unsigned char level_3;
		unsigned char level_4;

	};

	class ccIndicatorPage {
	public:
		virtual ~ccIndicatorPage();
		virtual void initialize(ed::basic_string<char> const& file, bool chunk = false);
		virtual void draw(Graphics::ModelParser&, unsigned char);
		virtual void addElem(Element*);
		virtual void update();
		virtual void update(unsigned char);
		virtual void setSize(float, float);
		virtual void setViewDistance(float);
		virtual int getType() const;
		virtual void setCurrentVertexScaleModifier(char, float);
		virtual float getCurrentVertexScaleModifier() const;
		virtual void setParent(ccDrawable*);
		virtual ccDrawable* getparent() const;
	protected:
		virtual void reg_in_script(lua_State*);
		virtual void reg_in_reusable_state(lua_State*);
		virtual void distribute_elements();

	public:
		ccIndicatorPage(ccIndicatorPage const&);
		ccIndicatorPage();
		std::unordered_map<ed::basic_string<char>, Element*>& get_SortedPageElems();
	protected:
		static int l_push_guid_string(lua_State*);
	public:
		void log(ed::basic_string<char>&);
		char const* nextElement();
		ccIndicatorPage& operator=(ccIndicatorPage const&);

		int nextElement_related;
		std::unordered_map<ed::basic_string<char>, Element*> pageElems;
		ed::vector<Element*> elements;
		float size_x;
		float size_y;
		float viewDistance;
		float currentVertexScaleModifier;
		ccDrawable* parent;

		//size 0x80
	};
	struct opacity_material {
		Graphics::Material* material;
		float initial_opacity;
	};

	class ccIndicationRenderParser {
	public:
		ccIndicationRenderParser(ccIndicationRenderParser&&);
		ccIndicationRenderParser(ccIndicationRenderParser const&);
		ccIndicationRenderParser(void);
		~ccIndicationRenderParser(void);
		void change_brightness(float);
		void change_color(osg::Vec3f const&);
		void change_opacity(float);
		void clear_counter(void);
		Graphics::MaterialPtr get_target_material(void)const;
		void increase_render_target_counter(void);
		bool is_inverted(void)const;
		ccIndicationRenderParser& operator=(ccIndicationRenderParser&&);
		ccIndicationRenderParser& operator=(ccIndicationRenderParser const&);
		void read_from_state(Lua::Config&);
		void release(void);
		void render(Graphics::RenderObject*);
		int rendered_with_targetmaterial(void)const;
		void restore_initial_color(void);
		void set_colored(bool);
		void set_inverted(bool);
		void set_mfd_shader_brightness(float);
		void set_mfd_shader_contrast(float);
		void set_shader_constant(float, float, float, float);
		void set_target_material(Graphics::MaterialPtr);
		void submit(class Graphics::RenderObject*);
		void beforeObjectRender(Graphics::RenderObject*);
		void set_reset_renderer_transform(bool val) { reset_renderer_transform = val; }

	protected:
		Graphics::MaterialPtr target_material;
		bool colored;
		int counter;
		float mfd_shader_brightness;
		float mfd_shader_contrast;
		bool inverted;
		bool reset_renderer_transform;
		ed::list<opacity_material> opacity_sensitive_materials;
		ed::list<Graphics::Material*> color_sensitive_materials;
		ed::list<Graphics::Material*> brightness_sensitive_materials;
		float brightness;
		float contrast;

		//size 0x60
	};

	class ccIndicator : public ccDrawable {
	public:
		/*01*/	virtual ~ccIndicator();
		/*02*/	virtual void initialize(avDevice*, unsigned char, ed::basic_string<char> const&);
		/*03*/	virtual void post_initialize();
		/*04*/	virtual void release();
		/*05*/	virtual void draw();
		/*06*/	virtual void draw(unsigned char);
		/*07*/	virtual void draw_screenspace(bool);
		/*08*/	virtual void draw_to_viewport(indicator_viewport const&);
		/*09*/
		/*10*/	virtual bool is_draw_screenspace() const;
		/*11*/	virtual void update();
		/*12*/	virtual void update(unsigned char);
		/*13*/	virtual void control();
		/*14*/
		/*15*/
		/*16*/
		/*17*/	virtual void push_purpose(int);
		/*18*/	virtual void pop_purpose();
		/*19*/	virtual void remove_purpose(int);
		/*20*/	virtual void set_geometry(indicator_geometry const&);
		/*21*/	virtual Graphics::Camera const& get_camera();
		/*22*/	virtual osg::Vec3d get_camera_point();
		/*23*/	virtual TVScreenCondition get_screen_condition();
		/*24*/	virtual void get_screen_params(HeapVector<Graphics::DynamicParam>&);
		/*25*/	virtual bool get_render_target_always() const;
		/*26*/	virtual void draw_temporal_set(Element*, unsigned int, unsigned char);
		/*27*/	virtual int getType() const;
		/*28*/	virtual std::unordered_map<ed::basic_string<char>, void (*)(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&), std::hash<ed::basic_string<char>>, std::equal_to<ed::basic_string<char>>, ed::allocator<std::pair<ed::basic_string<char> const, void(*)(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&)>>> const& get_elements_controllers() const;
		/*29*/	virtual std::unordered_map<ed::basic_string<char>, void (*)(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&), std::hash<ed::basic_string<char>>, std::equal_to<ed::basic_string<char>>, ed::allocator<std::pair<ed::basic_string<char> const, void(*)(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&)>>>& get_elements_controllers_ref();
		/*30*/	virtual std::unordered_map < ed::basic_string<char>, std::function<void(Element*, HeapVector<Graphics::DynamicParam>&)>, std::hash<ed::basic_string<char>>, std::equal_to<ed::basic_string<char>>, ed::allocator<std::pair<ed::basic_string<char> const, std::function<void(Element*, HeapVector<Graphics::DynamicParam>&)>>>> const& get_elements_controllers_lambdas() const;
		/*31*/	virtual std::unordered_map < ed::basic_string<char>, std::function<void(Element*, HeapVector<Graphics::DynamicParam>&)>, std::hash<ed::basic_string<char>>, std::equal_to<ed::basic_string<char>>, ed::allocator<std::pair<ed::basic_string<char> const, std::function<void(Element*, HeapVector<Graphics::DynamicParam>&)>>>>& get_elements_controllers_lambdas_ref();
		/*32*/	virtual int const get_render_target() const;
		/*33*/	virtual Element* get_element_on_current_page(char const*);
		/*34*/	virtual wPosition3<float> const& get_position(bool) const;
		/*35*/	virtual wPosition3<float> const& get_initpos() const;
	protected:
		/*36*/	virtual void RegisterInScript(lua_State*); //_guard_check_icall
	public:
		/*37*/	virtual void check_page(Device_Mode);
		/*38*/	virtual void create_elements_controllers_map();
	protected:
		/*39*/	virtual void initPages();
		/*40*/	virtual void PrepareParser(); //_guard_check_icall = 0
	protected:
		/*01*/	virtual void l_register_functions(struct lua_State*, int);
	public:
		static void blinking(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void blinking_direct(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void bound_by_box(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void bound_by_circle(osg::Vec3f&, float, float);
		static void bound_by_circle(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void bound_by_circle_without_root(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void change_color_when_parameter_equal_to_number(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void change_texture_state_using_parameter(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void compare_parameters(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void draw_argument_in_range(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void external_view_control(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void fov_control(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void increase_render_target_counter(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void line_object_set_point_using_parameters(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void move_element(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void move_left_right_using_parameter(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void move_up_down_using_parameter(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void opacity_using_parameter(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void parameter_compare_with_number(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void parameter_in_range(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void remove_orientation(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void render_purpose(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void rotate_element(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void rotate_using_parameter(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void scale_element(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void screenspace_position_control(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void text_using_parameter(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void txt_lo_resource(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void txt_lo_serialnumber(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void txt_lo_version(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void txt_txt_control(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void txt_UTF8_substr(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static void set_draw_boolean(Element*, HeapVector<Graphics::DynamicParam> const&, bool);
		static void set_draw_by_enum(Element*, HeapVector<Graphics::DynamicParam> const&, unsigned int);
		static void utility_set_origin_to_cockpit_shape(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&);
		static bool to_screen_space(osg::Vec3d const&, float&, float&, bool);
		static bool to_screen_space(osg::Vec3d const&, Element*, bool);
		static bool global_point_to_screen_space(osg::Vec3d const&, float&, float&, bool);
		static bool global_point_to_screen_space(osg::Vec3d const&, Element*, bool);
	public:
		ccIndicator(ccIndicator const&);
		ccIndicator();
		bool check_purpose(int) const;
		osg::Vec3f const get_collimator_default_distance_factor() const;
		ccIndicatorPage* get_page_by_ID(unsigned char);
		int get_render_purpose() const;
		ed::vector<ccIndicatorPage*>* GetCurrentPage();
		float GetHalfHeight() const;
		float GetHalfWidth() const;
		indicator_viewport const* getViewport() const;
		void initGeometry();
		void log_current_page(ed::basic_string<char>&);
		ccIndicator& operator=(ccIndicator const&);
		void set_collimator_distance_factor_modifier(osg::Vec3f const&);
		void set_render_target_always(bool);
		void SetCurrentPage(unsigned char);
	protected:
		void set_render_materials();
		void update_geometry();
		void update_position(int, indicator_viewport const*);
		ccIndicatorPage* createNewSubset(unsigned char, ed::basic_string<char> const&, bool);
		int get_render_purpose(bool) const;
	private:
		bool can_be_rendered_to_viewport() const;
		bool check_purpose_update(int) const;
		bool rendered_as_part_of_scene(int);
		void set_full_view_port_coverage(indicator_viewport const&);


	public:
		void make_draggable_frame(char const*, int, int, int, int);


	protected:
		gui::Window* window;
		ed::vector<ccIndicatorPage*>* currentPage;
		ed::map<unsigned char, ccIndicatorPage*> page_subsets;
		ed::map<unsigned char, ed::vector<ccIndicatorPage*>> pages;
		Graphics::Camera camera;
		TVScreenCondition screen_condition;
		wPosition3<float> initpos;
		wPosition3<float> position;
		wPosition3<float> viewportPosition;
		float halfWidth;
		float halfHeight;
		char indicator_type;
		int render_target;
		ccIndicationRenderParser indicationRenderParser;
		ed::basic_string<char> used_render_mask;
		bool render_target_always;
		ed::unordered_map<ed::basic_string<char>, void (*)(Element*, ccDrawable*, HeapVector<Graphics::DynamicParam> const&)> element_controllers;
		ed::vector<int> purposes;
		indicator_viewport dedicated_viewport;
		indicator_viewport dedicated_viewport_arcade;
		bool dynamically_update_geometry;
		indicator_geometry geometry;
		bool screen_space_now;
		osg::Vec3f collimator_default_distance_factor;
		bool collimator_default_distance_factor_bool;
		osg::Vec3f collimator_distance_factor_modifier;
		float some_float_1;
		std::unordered_map<ed::basic_string<char>, std::function<void(Element*, HeapVector<Graphics::DynamicParam>&)>> elements_controllers_lambdas;

		//size 0x660 2.9
	};

}
