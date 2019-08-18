#! /usr/bin/env python
"""
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	# title_name		: MultiLanguage System
	# date_created		: 2017.10.21
	# filename			: uiLanguageSystem.py
	# author			: VegaS ♆
	# version_actual	: Version 0.1.0
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
"""
import app
import ui
import uiScriptLocale
import uiToolTip
import mouseModule
import constInfo
import localeInfo
import net
import item
import uiGuild
import grp
import chat
import wndMgr
import sys
import dbg
import ime
import uiCommon

LOCALE_MAX_TEXT_LENGTH = \
[
	app.NOTICE_MAX_LEN,
	app.BIG_NOTICE_MAX_LEN,
	app.MAP_NOTICE_MAX_LEN,
	app.WHISHPER_MAX_LEN
]

LOCALE_DESCRIPTION_LANGUAGE = \
[
	localeInfo.LOCALE_LANGUAGE_EN, localeInfo.LOCALE_LANGUAGE_DE, localeInfo.LOCALE_LANGUAGE_RO,
	localeInfo.LOCALE_LANGUAGE_IT, localeInfo.LOCALE_LANGUAGE_CZ, localeInfo.LOCALE_LANGUAGE_PL,
	localeInfo.LOCALE_LANGUAGE_FR, localeInfo.LOCALE_LANGUAGE_HU, localeInfo.LOCALE_LANGUAGE_TR, 
	localeInfo.LOCALE_LANGUAGE_ES, localeInfo.LOCALE_LANGUAGE_PT, localeInfo.LOCALE_LANGUAGE_GL,
]

LOCALE_PATH_FLAG			= "d:/ymir work/ui/language_system/chat_language/%s.tga"
LOCALE_LANGUAGE_LIST		= app.GetLanguageList()
LOCALE_LANGUAGE_LIST.append(app.LANGUAGE_INDEX_GLOBAL)

def CanLoadLocaleFileByLanguage():
	return app.GetLanguage() <> app.MAIN_LOCALE_LANGUAGE

def ReloadClientInterface(type):
	def SendParseException(object, error, type):
		dbg.TraceError('Failed to load module: #{0}#'.format(object))
		if (error):
			dbg.TraceError('Error: #{0}#'.format(error))
		import exception
		exception.Abort(type)

	sysModuleDict = {
		app.CLIENT_FROM_LOGIN_MODULE: ['introLogin', 'localeInfo', 'uiScriptLocale'],
		app.CLIENT_FROM_GAME_MODULE: ['game', 'localeInfo', 'introLoading', 'uiScriptLocale', 'uiGameButton', 'uiMapNameShower', 'uiAffectShower', 'uiPlayerGauge', 'uiHelp'],
		app.CLIENT_FROM_INTERFACE_MODULE: ['localeInfo', 'uiScriptLocale', 'interfaceModule', 'uiToolTip']
	}
	
	if (not sysModuleDict.has_key(type)):
		dbg.TraceError('Usage call syntax: ReloadClientInterface({0})'.format(sysModuleDict.keys()))
		return

	for (key, object) in enumerate(sysModuleDict[type]):
		if (sys.modules.has_key(object)):
			try:
				if (sys.modules[object] <> None):
					del sys.modules[object]
					sys.modules.pop(object)

					execfile(object, {})
					__import__(object)
					dbg.TraceError('Succes reloading module: #{0}#'.format(object))

			except IOError, err:
				SendParseException(object, err, 'IOError')
			except RuntimeError, err:
				SendParseException(object, err, 'RuntimeError')
			except:
				SendParseException(object, None, 'LoadModuleFile')

class AnnouncementWindow(ui.ScriptWindow):
	BOARD_WIDTH				= 63
	X_START					= 10
	X_ADD_SPACE				= 40
	Y_START 				= 52
	Y_PARSER 				= 17
	WIDTH_WINDOW 			= X_ADD_SPACE * (app.LANGUAGE_MAX_NUM + 1)

	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.selectType		= 0
		self.selectLanguage = 0
		self.__LoadWindow()

	def __del__(self):
		ui.ScriptWindow.__del__(self)

	def __LoadWindow(self):
		try:
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, "uiscript/AnnouncementWindow.py")
		except:
			import exception
			exception.Abort("AnnouncementDialog.Initialize.LoadObject")
		try:
			GetObject = self.GetChild
			self.children = {
				'background' : {
					'color_reversed' : GetObject("bg1"),
					'color_black' : GetObject("bg2"),
					'bar' : GetObject("horizontalbar")},
				
				'main' : {
					'board' : GetObject("board"),
					'titleBar' : GetObject("titlebar"),
					'text_value' : GetObject("currentLine_Value"),
					'textLimitChars' : GetObject("textLine1"),
					'textLangSelected' : GetObject("textLine2"),
					'buttonList' : [GetObject("btnNotice"), GetObject("btnBigNotice"), GetObject("btnMapNotice"), GetObject("btnWhisper")],
					'checkBox' : {}},
				
				'events' : [
					GetObject("accept_button").SetEvent(self.AskQuestion),
					GetObject("clear_button").SetEvent(self.SetFocus)]
			}

		except:
			import exception
			exception.Abort("AnnouncementDialog.Initialize.BindObject")
		
		for i in xrange(app.LANGUAGE_MAX_NUM + 1):
			self.GetObject('main','checkBox').update({i: (
					ui.AnnouncementCheckBoxLanguage(self.GetObject('main','board'), \
						self.X_START + (self.X_ADD_SPACE * i), self.Y_START, lambda filter = i: self.OnSelectLanguage(filter)), \
					ui.MakeImageBox(self, LOCALE_PATH_FLAG % LOCALE_LANGUAGE_LIST[i], \
						(self.X_START * 2) + (self.X_ADD_SPACE * i), self.Y_START - self.Y_PARSER))
			})

		self.GetObject('main','titleBar').SetWidth(self.WIDTH_WINDOW)
		self.GetObject('main','titleBar').SetCloseEvent(self.Close)

		self.GetObject('main','text_value').SetSize(self.WIDTH_WINDOW, self.GetObject('main','text_value').GetHeight())
		self.GetObject('main','text_value').SetLimitWidth(self.WIDTH_WINDOW)
		self.GetObject('main','board').SetSize(self.WIDTH_WINDOW + self.Y_PARSER, self.GetObject('main','board').GetHeight())

		self.GetObject('background','color_reversed').SetSize(self.WIDTH_WINDOW, self.GetObject('background','color_reversed').GetHeight())
		self.GetObject('background','color_black').SetSize(self.WIDTH_WINDOW, self.GetObject('background','color_black').GetHeight())
		self.GetObject('background','bar').SetWidth(self.WIDTH_WINDOW)
		
		for (tabKey, tabButton) in enumerate(self.GetObject('main','buttonList')):
			tabButton.SAFE_SetEvent(self.ClickRadioButton, tabKey)
		
	def SetFocus(self):
		self.GetObject('main','text_value').SetText(app.EMPTY_STRING)
		self.GetObject('main','text_value').SetFocus()
		
	def GetObject(self, objects, keys):
		return self.children[objects][keys]
		
	def GetObjectConverted(self, bType):
		findObject, maxLimit = self.GetObject('main','text_value'), LOCALE_MAX_TEXT_LENGTH[bType]
		findObject.SetMax(maxLimit)
		return findObject.GetText()[0 : maxLimit]
		
	def ClickRadioButton(self, bType):
		for eachButton in self.GetObject('main','buttonList'):
			eachButton.SetUp()

		self.selectType = bType
		self.GetObject('main','buttonList')[bType].Down()
		self.GetObject('main','text_value').SetText(self.GetObjectConverted(bType))
		
		ime.SetCursorPosition(len(self.GetObject('main','text_value').GetText()) + 1)

	def OnSelectLanguage(self, bLanguage):
		for i in xrange(app.LANGUAGE_MAX_NUM + 1):
			if bLanguage == i:
				self.selectLanguage = bLanguage
				self.GetObject('main','textLangSelected').SetText(localeInfo.ANNOUNCEMENT_MANAGER_LANGUAGE_SELECTED.format(LOCALE_DESCRIPTION_LANGUAGE[self.selectLanguage]))
				self.GetObject('main','checkBox')[i][0].SetCheck(app.LANGUAGE_SELECTED)
			else:
				self.GetObject('main','checkBox')[i][0].SetCheck(app.LANGUAGE_UNSELECTED)

	def OnUpdate(self):
		self.GetObject('main','textLimitChars').SetText(localeInfo.ANNOUNCEMENT_MANAGER_LENGTH_TEXT.format(len(self.GetObject('main','text_value').GetText()), LOCALE_MAX_TEXT_LENGTH[self.selectType]))
	
		(mouseX, mouseY) = wndMgr.GetMousePosition()
		for i in xrange(app.LANGUAGE_MAX_NUM + 1):
			if self.GetObject('main','checkBox')[i][0].IsIn():
				self.toolTip = uiToolTip.ToolTip()
				self.toolTip.SetPosition(mouseX + 25, mouseY)
				self.toolTip.AppendDescription(LOCALE_DESCRIPTION_LANGUAGE[i], None, 0xffffa879)

	def Destroy(self):
		self.ClearDictionary()
		self.children.clear()
		
	def Open(self):
		self.OnSelectLanguage(app.LANGUAGE_MAX_NUM)
		self.ClickRadioButton(0)
		self.SetCenterPosition()
		self.UpdateRect()
		self.SetFocus()
		self.Show()
		
	def SendPacketAnnouncement(self):

		def GetType():
			return self.selectType

		def GetText():
			return self.GetObject('main','text_value').GetText()

		def GetLanguage():
			baseDict = {i : app.LANGUAGE_UNSELECTED for i in xrange(app.LANGUAGE_MAX_NUM + 1)}
			baseDict.update({0 : self.selectLanguage})
			return tuple(baseDict.values())

		app.SendShoutLanguageFilterPacket(app.LANGUAGE_SUB_HEADER_GM_ANNOUNCEMENT, GetType(), GetText(), GetLanguage())
		
	def AnswerOnQuestion(self, answer):
		if not self.wndQuestionDialog:
			return

		self.wndQuestionDialog.Close()
		self.wndQuestionDialog = None
		
		if answer:		
			self.SendPacketAnnouncement()
		
	def AskQuestion(self):
		self.wndQuestionDialog = uiCommon.QuestionDialog()
		self.wndQuestionDialog.SetText(localeInfo.ANNOUNCEMENT_MANAGER_QUESTION_SEND)
		self.wndQuestionDialog.SetWidth(300)
		self.wndQuestionDialog.SetAcceptEvent(lambda arg = TRUE: self.AnswerOnQuestion(arg))
		self.wndQuestionDialog.SetCancelEvent(lambda arg = FALSE: self.AnswerOnQuestion(arg))
		self.wndQuestionDialog.Open()
		
	def Close(self):
		self.Hide()
		
	def OnPressEscapeKey(self):
		self.Close()
		return TRUE

class ChatFilterWindow(ui.Window):
	WND_WIDTH			= 63
	WND_HEIGHT			= 184
	WND_TOOLTIP_WIDTH	= 125
	X_PARSER_TOOLTIP	= 35
	X_START				= 42
	Y_SPACE				= 18
	ADD_SPACE			= 3

	def __init__(self):
		ui.Window.__init__(self)
		self.checkBoxDataList, self.imageBoxList = [], []
		self.__LoadWindow()

	def __del__(self):
		ui.Window.__del__(self)
		
	def AddImageBox(self, x, y, fileName):
		return self.imageBoxList.append(ui.MakeImageBox(self, fileName, x, y))
		
	def GetGlobalPositionByChatParent(self, chatPosX, chatPosY):
		self.SetPosition(chatPosX - self.WND_WIDTH, chatPosY - self.Y_SPACE * (app.LANGUAGE_MAX_NUM + 1))
		self.UpdateRect()

	def __LoadWindow(self):
		for i in xrange(app.LANGUAGE_MAX_NUM + 1):
			self.checkBoxDataList.append(ui.ChatFilterCheckBoxLanguage(self, i, 0, (self.Y_SPACE * i), lambda filter = i: self.OnSelectLanguage(filter)))
			self.AddImageBox(self.X_START, (self.Y_SPACE * i) + self.ADD_SPACE, LOCALE_PATH_FLAG % LOCALE_LANGUAGE_LIST[i])

		self.SetSize(self.WND_WIDTH, self.Y_SPACE * (app.LANGUAGE_MAX_NUM + 1))
		self.Show()

	def OnSelectLanguage(self, bLanguageID):
		def Reload():
			map(ui.ChatFilterCheckBoxLanguage.UnSelect, self.checkBoxDataList)

		for i in xrange(app.LANGUAGE_MAX_NUM + 1):
			if (bLanguageID <> i):
				continue

			checkBox = self.checkBoxDataList[i]
			if (self.checkBoxDataList[app.LANGUAGE_MAX_NUM].IsChecked()):
				Reload()
			
			if (bLanguageID == app.LANGUAGE_MAX_NUM):
				Reload()
				checkBox.Select()
				self.OnCheckLanguages(True)
				return

			if (checkBox.IsChecked()):
				checkBox.UnSelect()
			else:
				checkBox.Select()

		self.OnCheckLanguages()
				
	def OnCheckLanguages(self, bLanguageGlobal = False):
		baseDict = {i : app.LANGUAGE_UNSELECTED for i in xrange(app.LANGUAGE_MAX_NUM + 1)}

		if (bLanguageGlobal):
			baseDict.update({app.LANGUAGE_MAX_NUM : app.LANGUAGE_SELECTED})
			app.SendShoutLanguageFilterPacket(app.LANGUAGE_SUB_HEADER_FILTER_CHAT, 0, app.EMPTY_STRING, tuple(baseDict.values()))
			return

		for i in xrange(app.LANGUAGE_MAX_NUM):
			if self.checkBoxDataList[i].IsChecked():
				baseDict.update({i : app.LANGUAGE_SELECTED})

		app.SendShoutLanguageFilterPacket(app.LANGUAGE_SUB_HEADER_FILTER_CHAT, 0, app.EMPTY_STRING, tuple(baseDict.values()))

	def OnUpdate(self):
		(mouseX, mouseY) = wndMgr.GetMousePosition()

		for i in xrange(app.LANGUAGE_MAX_NUM + 1):
			if self.checkBoxDataList[i].IsIn():
				self.toolTipDialog = uiToolTip.ToolTip()
				self.toolTipDialog.SetPosition(mouseX + self.X_PARSER_TOOLTIP, mouseY)
				self.toolTipDialog.RectSize(self.WND_TOOLTIP_WIDTH, 0)
				self.toolTipDialog.AppendDescription(LOCALE_DESCRIPTION_LANGUAGE[i], None, 0xffffa879)	
				
	def Destroy(self):
		self.checkBoxDataList = []
		self.imageBoxList = []

	def LoadShoutFilterList(self, bLanguageID, bValue):
		if (bLanguageID > len(self.checkBoxDataList) and bValue not in (app.LANGUAGE_SELECTED, app.LANGUAGE_UNSELECTED)):
			return

		if (bValue == app.LANGUAGE_SELECTED):
			self.checkBoxDataList[bLanguageID].Select()
		else:
			self.checkBoxDataList[bLanguageID].UnSelect()