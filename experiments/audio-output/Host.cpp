#include "HostAPI.h"
#include "tier1/tier1.h"
#include "tier2/tier2.h"
#include "tier3/tier3.h"
#include "vgui_controls/Controls.h"
#include "vgui_controls/AnimationController.h"
#include "vgui_controls/PropertyDialog.h"
#include "vgui_controls/PropertyPage.h"
#include "vgui_controls/PropertySheet.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/PHandle.h"
#include "vgui/IInput.h"
#include "vgui/ISurface.h"
#include "vgui/IScheme.h"
#include "tier1/KeyValues.h"
#include "tier0/dbg.h"
#include "mathlib/mathlib.h"
#include "tier0/memdbgon.h"
namespace {
bool polishUI=false;
void InspectRender(vgui::Panel *panel,int traversals,int backgrounds,int paints){
 int x,y,w,h,cx,cy,cw,ch;panel->GetBounds(x,y,w,h);panel->GetClipRect(cx,cy,cw,ch);
 int popupIndex=-1;
 for(int i=0;i<vgui::surface()->GetPopupCount();++i)if(vgui::surface()->GetPopup(i)==panel->GetVPanel())popupIndex=i;
 Msg("[VguiRender] %s traversal=%d background=%d paint=%d alpha=%d scheme=%u visible=%d popup=%d popupIndex=%d/%d topmost=%d bounds=%d,%d,%d,%d clip=%d,%d,%d,%d surfaceAlpha=%g\n",
 panel->GetName(),traversals,backgrounds,paints,panel->GetAlpha(),(unsigned)panel->GetScheme(),panel->IsVisible(),panel->IsPopup(),
 popupIndex,vgui::surface()->GetPopupCount(),vgui::surface()->GetTopmostPopup()==panel->GetVPanel(),x,y,w,h,cx,cy,cw,ch,vgui::surface()->DrawGetAlphaMultiplier());
}
class RenderProbe:public vgui::Panel {
 DECLARE_CLASS_SIMPLE(RenderProbe,vgui::Panel);
public:
 RenderProbe(vgui::VPANEL parent):BaseClass(NULL,"OpaqueHostRenderProbe"),traversals(0),backgrounds(0),paints(0){
 SetParent(parent);SetBounds(330,190,620,220);
 SetPaintEnabled(true);SetPaintBackgroundEnabled(true);SetAlpha(255);
 SetBgColor(Color(0,50,100,255));SetVisible(true);SetEnabled(true);
 MakePopup(false); // Deliberately non-modal, minimal same-module popup.
 vgui::Label *label=new vgui::Label(this,"StaticLabel","HOST-ONLY RENDER PROBE - opaque blue with yellow border");
 label->SetBounds(20,35,580,60);label->SetFgColor(Color(255,255,255,255));
 MoveToFront();Repaint();
 Msg("[VguiRender] created host-only probe; no client receiver\n");
 }
 void PaintTraverse(bool repaint,bool allowForce=true) OVERRIDE {
  if(++traversals==1)Msg("[VguiRender] probe PaintTraverse reached\n");
  BaseClass::PaintTraverse(repaint,allowForce);
 }
 void PaintBackground() OVERRIDE {
  if(++backgrounds==1)Msg("[VguiRender] probe PaintBackground reached alpha=%g\n",vgui::surface()->DrawGetAlphaMultiplier());
  vgui::surface()->DrawSetColor(0,50,100,255);vgui::surface()->DrawFilledRect(0,0,GetWide(),GetTall());
 }
 void Paint() OVERRIDE {
  if(++paints==1)Msg("[VguiRender] probe Paint reached\n");
  vgui::surface()->DrawSetColor(255,230,0,255);
  for(int i=0;i<4;++i)vgui::surface()->DrawOutlinedRect(i,i,GetWide()-i,GetTall()-i);
 }
 void Inspect(){InspectRender(this,traversals,backgrounds,paints);}
 int traversals,backgrounds,paints;
};
RenderProbe *renderProbe=NULL;
class HostDialog;
class AudioPage:public vgui::PropertyPage {
 DECLARE_CLASS_SIMPLE(AudioPage,vgui::PropertyPage);
public:
 AudioPage(vgui::Panel *parent):BaseClass(parent,"Audio"){
 KeyValues *settings=new KeyValues("Layout");settings->SetInt("skip_autoresize",1);ApplySettings(settings);settings->deleteThis();
 }
 void PerformLayout() OVERRIDE {
 BaseClass::PerformLayout();
 vgui::Panel *mount=FindChildByName("CrossModuleMount");
 vgui::Panel *reset=FindChildByName("ResetProbe");
 if(mount)mount->SetBounds(16,12,580,290);
 if(reset)reset->SetBounds(16,310,260,26);
 }
 void OnResetData() OVERRIDE;
 void OnApplyChanges() OVERRIDE {Msg("[VguiHost] PAGE ApplyChanges (receiver already acknowledged)\n");}
};
class HostDialog:public vgui::PropertyDialog {
 DECLARE_CLASS_SIMPLE(HostDialog,vgui::PropertyDialog);
public:
 HostDialog(vgui::VPANEL parent):BaseClass(NULL,"AudioTransactionHost"),busy(false),traversals(0),backgrounds(0),paints(0),thinks(0) {
 Msg("[VguiAlpha] after PropertyDialog construction alpha=%d\n",GetAlpha());
 SetParent(parent);SetDeleteSelfOnClose(false);SetSize(640,460);SetTitle(polishUI?L"Wybierz urz\u0105dzenie audio":L"Select Audio Output",true);
 page=new AudioPage(this);AddPage(page,"Audio");
 mount=new vgui::Panel(page,"CrossModuleMount"); mount->SetBounds(16,12,580,290);
 vgui::Label *label=new vgui::Label(page,"Notice","");
 label->SetBounds(20,20,540,24);
 vgui::Button *reset=new vgui::Button(page,"ResetProbe","Reset pending selection",this,"ResetProbe");
 reset->SetText(polishUI?L"Resetuj oczekuj\u0105cy wyb\u00f3r":L"Reset pending selection");reset->SetBounds(16,310,260,26);
 SetApplyButtonVisible(true);Msg("[VguiAlpha] before Activate alpha=%d\n",GetAlpha());Activate();Msg("[VguiAlpha] after Activate alpha=%d\n",GetAlpha());MoveToCenterOfScreen();
 Msg("[VguiHost] CREATE module=%s page=%u\n",vgui::GetControlsModuleName(),(unsigned)page->GetVPanel());
 }

 void ApplySchemeSettings(vgui::IScheme *scheme) OVERRIDE {
  Msg("[VguiAlpha] scheme before alpha=%d transition=%s\n",GetAlpha(),scheme->GetResourceString("Frame.TransitionEffectTime"));
  BaseClass::ApplySchemeSettings(scheme);
  Msg("[VguiAlpha] scheme after alpha=%d\n",GetAlpha());
 }
 void OnThink() OVERRIDE {
  int before=GetAlpha();BaseClass::OnThink();++thinks;
  if(thinks<=4||thinks==60||before!=GetAlpha())
   Msg("[VguiAlpha] think=%d before=%d after=%d\n",thinks,before,GetAlpha());
 }
 void AlphaOnce(){Msg("[VguiAlpha] diagnostic one-shot before=%d\n",GetAlpha());SetAlpha(255);Repaint();Msg("[VguiAlpha] diagnostic one-shot after=%d\n",GetAlpha());}
 bool Bind(vgui::VPANEL p) {if(busy)return false;Discard();receiver=p;return ResetReceiver();}
 int Request(int operation) {
 if(busy || !receiver.Get()) return -1;
 busy=true;
 KeyValues *kv=new KeyValues("AudioHostTransactionV1");kv->SetInt("operation",operation);kv->SetInt("result",-1);
 Msg("[VguiHost] REQUEST begin operation=%d\n",operation);
 bool handled=vgui::ipanel()->RequestInfo(receiver.Get(),kv);
 int result=handled?kv->GetInt("result",-1):-1;kv->deleteThis();
 busy=false;Msg("[VguiHost] REQUEST end operation=%d result=%d handled=%d\n",operation,result,handled);return result;
 }
 bool ResetReceiver(){int result=Request(1);return result==0;}
 void Discard(){if(receiver.Get())Request(3);receiver=static_cast<vgui::VPANEL>(0);Msg("[VguiHost] UNBIND\n");}
 ~HostDialog(){Discard();Msg("[VguiHost] DESTROY\n");}
 void OnCommand(const char *command) OVERRIDE {
 Msg("[VguiHost] COMMAND %s\n",command);
 if(busy){Msg("[VguiHost] rejected reentrant command\n");return;}
 if(!V_stricmp(command,"ResetProbe")){GetPropertySheet()->ResetAllData();return;}
 if(!V_stricmp(command,"Apply")||!V_stricmp(command,"OK")){
 int result=Request(2);
 if(result!=0&&result!=1){EnableApplyButton(true);SetTitle(polishUI?L"Nie zastosowano wyboru urz\u0105dzenia":L"Audio selection could not be applied",true);return;}
 }
 if(!V_stricmp(command,"Cancel")||!V_stricmp(command,"Close"))Discard();
 BaseClass::OnCommand(command);
 }
 void OnClose() OVERRIDE { if(busy)return; Discard(); Msg("[VguiHost] OnClose explicit discard/hide\n"); BaseClass::OnClose(); }
 void PaintTraverse(bool repaint,bool allowForce=true) OVERRIDE {
  if(++traversals==1)Msg("[VguiRender] dialog PaintTraverse reached\n");
  BaseClass::PaintTraverse(repaint,allowForce);
 }
 void PaintBackground() OVERRIDE {
  if(++backgrounds==1)Msg("[VguiRender] dialog PaintBackground reached\n");
  BaseClass::PaintBackground();
 }
 void Paint() OVERRIDE {
  if(++paints==1)Msg("[VguiRender] dialog Paint reached\n");
  BaseClass::Paint();
 }
 void Inspect() {
  InspectRender(this,traversals,backgrounds,paints);
  int x,y,w,h;GetBounds(x,y,w,h);
  Msg("[VguiHost] STATE visible=%d popup=%d enabled=%d kb=%d mouse=%d bounds=%d,%d,%d,%d focus=%p modal=%p\n",
   IsVisible(),IsPopup(),IsEnabled(),IsKeyBoardInputEnabled(),IsMouseInputEnabled(),x,y,w,h,
   (void*)vgui::input()->GetFocus(),(void*)vgui::input()->GetAppModalSurface());
  for(vgui::VPANEL p=GetVPanel();p;p=vgui::ipanel()->GetParent(p))
   Msg("[VguiHost] ancestor name=%s visible=%d popup=%d\n",vgui::ipanel()->GetName(p),vgui::ipanel()->IsVisible(p),vgui::ipanel()->IsPopup(p));
  for(int i=0;i<page->GetChildCount();++i)
   Msg("[VguiHost] hotkey child=%d sameModule=%d\n",i,page->GetChild(i)!=NULL);
 }
 AudioPage *page; vgui::Panel *mount; bool busy; int traversals,backgrounds,paints,thinks; vgui::VPanelHandle receiver;
};
HostDialog *host=NULL;
void AudioPage::OnResetData(){Msg("[VguiHost] PAGE ResetData\n");if(host && host->receiver.Get())host->ResetReceiver();}
}
extern "C" __declspec(dllexport) bool AudioHostInit(CreateInterfaceFn factory){
 static bool initialized=false;if(initialized)return true;
 ConnectTier1Libraries(&factory,1);ConnectTier2Libraries(&factory,1);ConnectTier3Libraries(&factory,1);
 MathLib_Init(2.2f,2.2f,0.0f,2.0f);
 initialized=vgui::VGui_InitInterfacesList("audio_transaction_host",&factory,1);
 Msg("[VguiHost] INIT result=%d\n",initialized);return initialized;
}
extern "C" __declspec(dllexport) vgui::VPANEL AudioHostCreate(vgui::VPANEL parent){
 if(host)return 0;host=new HostDialog(parent);host->Inspect();return host->mount->GetVPanel();
}
extern "C" __declspec(dllexport) bool AudioHostBind(vgui::VPANEL p){return host&&host->Bind(p);}
extern "C" __declspec(dllexport) void AudioHostUnbind(){if(host&&!host->busy)host->Discard();}
extern "C" __declspec(dllexport) void AudioHostDestroy(){if(host&&!host->busy){HostDialog *old=host;host=NULL;delete old;} if(renderProbe){delete renderProbe;renderProbe=NULL;}}
extern "C" __declspec(dllexport) void AudioHostInspect(){if(host)host->Inspect();if(renderProbe)renderProbe->Inspect();}
extern "C" __declspec(dllexport) vgui::VPANEL AudioHostRenderProbe(vgui::VPANEL parent){
 if(host||renderProbe)return 0;renderProbe=new RenderProbe(parent);renderProbe->Inspect();return renderProbe->GetVPanel();
}
static void AudioHostAlphaOnce(){if(host)host->AlphaOnce();}
static void AudioHostUpdate(float time){vgui::GetAnimationController()->UpdateAnimations(time);}
static void AudioHostLanguage(bool polish){polishUI=polish;}
static void *GetHostAPI(){
 static AudioHostAPI api={AudioHostInit,AudioHostCreate,AudioHostBind,AudioHostUnbind,AudioHostDestroy,AudioHostInspect,AudioHostRenderProbe,AudioHostAlphaOnce,AudioHostUpdate,AudioHostLanguage};
 return &api;
}
EXPOSE_INTERFACE_FN(GetHostAPI,AudioHostAPI,"AudioTransactionHost001");







