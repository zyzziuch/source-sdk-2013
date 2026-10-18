#include "cbase.h"
#ifdef _WIN64
#include "HostAPI.h"
#include "client_factorylist.h"
#include "igamesystem.h"
#include "ienginevgui.h"
#include "vgui_controls/EditablePanel.h"
#include "vgui_controls/ComboBox.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/ListPanel.h"
#include "vgui_controls/Label.h"
#include "GameUI/AudioOutputWatchWin.h"
#include "vgui_controls/PHandle.h"
#include "GameUI/AudioOutputSelection.h"
#include "GameUI/AudioOutputDevicesWin.h"
#include "tier0/memdbgon.h"
namespace {
CSysModule *module=NULL; AudioHostAPI *api=NULL;
bool rejectNext=false;
class Receiver:public vgui::EditablePanel {
 DECLARE_CLASS_SIMPLE(Receiver,vgui::EditablePanel);
public:
 Receiver(vgui::VPANEL parent):BaseClass(NULL,"ClientAudioReceiver"),selection(cvar,engine,EnumerateAudioOutputDevicesWin,NULL),ready(false),busy(false),refreshAt(0){
 SetParent(parent);SetBounds(0,0,580,290);
 KeyValues *layout=new KeyValues("Layout");layout->SetInt("skip_autoresize",1);ApplySettings(layout);layout->deleteThis();
 char language[64];engine->GetUILanguage(language,sizeof(language));polish=!V_stricmp(language,"polish");
 combo=new vgui::ComboBox(this,"InternalSelection",8,false);combo->SetVisible(false);
 list=new vgui::ListPanel(this,"PlaybackDevices");list->SetMultiselectEnabled(false);
 list->AddColumnHeader(0,"name",polish?"Urządzenie odtwarzania":"Playback device",545);
 list->AddActionSignalTarget(this);
 refresh=new vgui::Button(this,"Refresh",polish?L"Od\u015bwie\u017c urz\u0105dzenia":L"Refresh Devices",this,"Refresh");
 status=new vgui::Label(this,"Status","");status->SetWrap(true);
 watch.Start();
 Msg("[VguiReceiver] CREATE module=%s\n",vgui::GetControlsModuleName());
 }
 ~Receiver(){watch.Stop();Msg("[VguiReceiver] DESTROY\n");}
 void PerformLayout() OVERRIDE {
 BaseClass::PerformLayout();list->SetBounds(0,0,580,215);refresh->SetBounds(0,225,210,26);status->SetBounds(0,255,580,32);
 }
 const char *Staged(){KeyValues *data=combo->GetActiveItemUserData();return data?data->GetString("override"):"";}
 void Feedback(const wchar_t *en,const wchar_t *pl){status->SetText(polish?pl:en);}
 bool Refresh(){
 CUtlString wanted(Staged());CUtlVector<AudioOutputChoice_t> choices;
 if(!EnumerateAudioOutputDevicesWin(NULL,choices)){Feedback(L"Cannot refresh devices. Previous list retained.",L"Nie mo\u017cna od\u015bwie\u017cy\u0107 listy. Zachowano poprzedni\u0105.");return false;}
 bool found=false;
 FOR_EACH_VEC(choices,i)if(!V_strcmp(choices[i].m_OverrideValue.Get(),wanted.Get()))found=true;
 if(!found){AudioOutputChoice_t missing;missing.m_OverrideValue=wanted;missing.m_DisplayName=polish?"Wybrane urządzenie (niedostępne)":"Selected device (unavailable)";choices.AddToTail(missing);}
 combo->RemoveAll();list->RemoveAll();int selected=-1;
 FOR_EACH_VEC(choices,i){
  AudioOutputChoice_t &choice=choices[i];CUtlString name(choice.m_DisplayName);
  CUtlString suffix(" [");suffix+=choice.m_OverrideValue.Get();suffix+="]";
  if(choice.m_OverrideValue.Length()&&name.Length()>=suffix.Length()&&!V_strcmp(name.Get()+name.Length()-suffix.Length(),suffix.Get())){
   CUtlVector<char> trimmed;trimmed.SetCount(name.Length()-suffix.Length()+1);V_strncpy(trimmed.Base(),name.Get(),trimmed.Count());name=trimmed.Base();
  }
  if(!choice.m_OverrideValue.Length())name=polish?"Domyślne urządzenie systemowe":"System Default";
  KeyValues *data=new KeyValues("Device");data->SetString("override",choice.m_OverrideValue.Get());data->SetBool("available",choice.m_bAvailable);
  int comboID=combo->AddItem(name.Get(),data);data->SetInt("combo",comboID);
  CUtlVector<wchar_t> wide;wide.SetCount(name.Length()+1);V_UTF8ToUnicode(name.Get(),wide.Base(),wide.Count()*sizeof(wchar_t));data->SetWString("name",wide.Base());
  int row=list->AddItem(data,0,false,false);data->deleteThis();
  list->SetItemDisabled(row,!choice.m_bAvailable);
  if(!V_strcmp(choice.m_OverrideValue.Get(),wanted.Get())){combo->SilentActivateItem(comboID);selected=row;}
 }
 if(selected>=0)list->SetSingleSelectedItem(selected);
 KeyValues *active=combo->GetActiveItemUserData();
 if(active&&!active->GetBool("available"))Feedback(L"Selected endpoint is unavailable. Preference preserved.",L"Wybrane urz\u0105dzenie niedost\u0119pne. Wyb\u00f3r zachowany.");
 else Feedback(L"System Default may require an audio restart.",L"Opcja domy\u015blna mo\u017ce wymaga\u0107 restartu d\u017awi\u0119ku.");
 Msg("[VguiReceiver] list refreshed; staged ID preserved; no preference write\n");return true;
 }
 bool RequestInfo(KeyValues *request) OVERRIDE {
 if(V_strcmp(request->GetName(),"AudioHostTransactionV1"))return BaseClass::RequestInfo(request);
 const int op=request->GetInt("operation");int result=-1;
 if(busy){request->SetInt("result",-2);return true;}
 busy=true;Msg("[VguiReceiver] REQUEST operation=%d\n",op);
 if(op==1){ready=selection.Reset(*combo);result=ready?CAudioOutputSelection::UNCHANGED:CAudioOutputSelection::UNSUPPORTED;if(ready)Refresh();}
 else if(op==2){
  if(rejectNext){rejectNext=false;result=CAudioOutputSelection::PREFERENCE_REJECTED;Msg("[VguiReceiver] TEST rejection; helper not called\n");}
  else result=ready?selection.Apply(*combo):CAudioOutputSelection::UNSUPPORTED;
  if(result>1)Feedback(L"Could not apply. Check connection/device, or Reset and retry.",L"Nie zastosowano. Sprawd\u017a urz\u0105dzenie/po\u0142\u0105czenie lub resetuj wyb\u00f3r.");
  else Feedback(L"Preference accepted.",L"Wyb\u00f3r zaakceptowany.");
 }else if(op==3){ready=false;result=CAudioOutputSelection::UNCHANGED;}
 request->SetInt("result",result);busy=false;
 Msg("[VguiReceiver] RESULT operation=%d result=%d\n",op,result);return true;
 }
 void OnThink() OVERRIDE {
 BaseClass::OnThink();selection.UpdateEnabled(*combo);list->SetEnabled(ready&&combo->IsEnabled());
 if(watch.Poll()&&!refreshAt)refreshAt=Plat_FloatTime()+0.35;
 if(ready&&!busy&&refreshAt&&Plat_FloatTime()>=refreshAt){refreshAt=0;Refresh();}
 }
 void OnCommand(const char *cmd) OVERRIDE {if(!V_strcmp(cmd,"Refresh")){if(ready&&!busy)Refresh();return;}BaseClass::OnCommand(cmd);}
private:
 MESSAGE_FUNC(OnChanged,"ItemSelected"){
 if(busy||!ready||list->GetSelectedItemsCount()!=1)return;
 KeyValues *row=list->GetItem(list->GetSelectedItem(0));if(!row||!row->GetBool("available"))return;
 if(!V_strcmp(row->GetString("override"),Staged()))return;
 combo->SilentActivateItem(row->GetInt("combo"));
 Msg("[VguiReceiver] STAGED ItemSelected; no preference write\n");
 vgui::VPANEL parent=GetVParent();
 while(parent&&V_strcmp(vgui::ipanel()->GetClassName(parent),"PropertySheet"))parent=vgui::ipanel()->GetParent(parent);
 if(parent)vgui::ipanel()->SendMessage(parent,new KeyValues("ApplyButtonEnable"),GetVPanel());
 }
 CAudioOutputSelection selection;CAudioOutputWatchWin watch;vgui::ComboBox *combo;vgui::ListPanel *list;vgui::Button *refresh;vgui::Label *status;
 bool ready,busy,polish;double refreshAt;
};
vgui::DHANDLE<Receiver> receiver;
void Destroy(){
 if(api)api->Unbind();
 Receiver *owned=receiver.Get();receiver=NULL;
 if(owned){owned->SetParent(static_cast<vgui::VPANEL>(0));delete owned;}
 if(api)api->Destroy();
 rejectNext=false;Msg("[VguiClient] panel cleanup complete (also called before opening); host DLL pinned until process exit\n");
}
bool Load(){
 if(api)return true;
 char path[MAX_PATH];V_snprintf(path,sizeof(path),"%s/bin/x64/audio_transaction_host.dll",engine->GetGameDirectory());
 module=Sys_LoadModule(path);
 if(!module){Warning("[VguiClient] module load failed: %s\n",path);return false;}
 CreateInterfaceFn factory=Sys_GetFactory(module);
 api=factory?static_cast<AudioHostAPI*>(factory("AudioTransactionHost001",NULL)):NULL;
 if(!api){Warning("[VguiClient] host ABI absent\n");return false;}
 factorylist_t factories;FactoryList_Retrieve(factories);
 if(!api->Init(factories.appSystemFactory)){api=NULL;Warning("[VguiClient] host init failed\n");return false;}
 char language[64];engine->GetUILanguage(language,sizeof(language));api->SetPolish(!V_stricmp(language,"polish"));
 return true;
}
void OpenAudioHost(){
 if(!Load())return;Destroy();
 vgui::VPANEL page=api->Create(enginevgui->GetPanel(PANEL_GAMEUIDLL));
 if(!page)return;
 receiver=new Receiver(page);bool bound=api->Bind(receiver->GetVPanel());
 Msg("[VguiClient] BIND result=%d\n",bound);
}
class AudioEntry:public vgui::Button {
 DECLARE_CLASS_SIMPLE(AudioEntry,vgui::Button);
public:
 AudioEntry(vgui::VPANEL parent):BaseClass(NULL,"ClientAudioOutputButton","Select Audio Output"){
  SetParent(parent);SetBounds(0,0,210,24);AddActionSignalTarget(this);SetCommand("OpenAudioOutputHost");
  char language[64];engine->GetUILanguage(language,sizeof(language));
  if(!V_stricmp(language,"polish"))SetText(L"Wybierz urz\u0105dzenie audio");
 }
 void OnCommand(const char *command) OVERRIDE {
  if(!V_strcmp(command,"OpenAudioOutputHost")){
   Msg("[AudioEntry] actual button Command received; opening host\n");OpenAudioHost();return;
  }
  BaseClass::OnCommand(command);
 }
};
vgui::DHANDLE<AudioEntry> entry;
vgui::VPANEL FindMount(vgui::VPANEL p,int depth,int &budget){
 if(!p||depth>32||--budget<0)return 0;
 if(!V_strcmp(vgui::ipanel()->GetName(p),"SDKAudioOutputMount"))return p;
 for(int i=0;i<vgui::ipanel()->GetChildCount(p);++i){
  vgui::VPANEL found=FindMount(vgui::ipanel()->GetChild(p,i),depth+1,budget);if(found)return found;
 }
 return 0;
}
void DiscoverEntry(){
 if(entry.Get())return;
 int budget=20000;vgui::VPANEL mount=FindMount(enginevgui->GetPanel(PANEL_GAMEUIDLL),0,budget);
 if(!mount)return;
 vgui::VPANEL page=vgui::ipanel()->GetParent(mount);bool speakers=false,volume=false;
 for(int i=0;page&&i<vgui::ipanel()->GetChildCount(page);++i){
  const char *name=vgui::ipanel()->GetName(vgui::ipanel()->GetChild(page,i));
  speakers|=!V_strcmp(name,"SpeakerSetup");volume|=!V_strcmp(name,"SFXSlider");
 }
 if(!speakers||!volume)return;
 entry=new AudioEntry(mount);Msg("[AudioEntry] attached client button under Audio resource mount\n");
}
void DestroyEntry(){AudioEntry *owned=entry.Get();entry=NULL;if(owned){owned->SetParent(static_cast<vgui::VPANEL>(0));delete owned;}}
class Lifetime:public CAutoGameSystemPerFrame {
public: Lifetime():CAutoGameSystemPerFrame("AudioHostRuntime"),stopped(false),nextDiscovery(0){}
 void Update(float) OVERRIDE {if(stopped)return;if(api)api->Update(engine->Time());if(Plat_FloatTime()>=nextDiscovery){nextDiscovery=Plat_FloatTime()+0.5;DiscoverEntry();}}
 void Shutdown() OVERRIDE {stopped=true;DestroyEntry();Destroy();}
 bool stopped; double nextDiscovery;
} lifetime;
}
CON_COMMAND(audio_host_open,"Open isolated audio host."){OpenAudioHost();}
CON_COMMAND(audio_host_destroy,"Destroy receiver and host panels; retain DLL for safe process lifetime."){Destroy();}
CON_COMMAND(audio_host_reject_next,"Reject the next test-host Apply/OK without calling the selector."){rejectNext=true;Msg("[VguiClient] next Apply/OK will reject without writing\n");}
CON_COMMAND(audio_host_rebind,"Reset/rebind the current receiver via host API."){
 if(api&&receiver.Get())Msg("[VguiClient] REBIND result=%d\n",api->Bind(receiver->GetVPanel()));
}
CON_COMMAND(audio_host_inspect,"Log host popup, visibility, focus and module-boundary state."){if(api)api->Inspect();}
CON_COMMAND(audio_host_render_probe,"Host-only opaque popup, no receiver."){
 if(!Load())return;Destroy();api->RenderProbe(enginevgui->GetPanel(PANEL_GAMEUIDLL));
}
CON_COMMAND(audio_host_dialog_only,"Original host dialog without a receiver."){
 if(!Load())return;Destroy();api->Create(enginevgui->GetPanel(PANEL_GAMEUIDLL));
}
CON_COMMAND(audio_host_alpha_once,"One-shot host alpha diagnostic; no audio writes."){if(api)api->AlphaOnce();}
#endif








