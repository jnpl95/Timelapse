#include "Settings.h"
#include "Log.h"

using namespace Timelapse;
using namespace System;
using namespace IO;
using namespace Collections::Generic;

Object^ Settings::Deserialize(String^ path, XmlSerializer^ serializer) {
	if (File::Exists(path)) {
		FileStream^ stream = {};

		try {
			stream = gcnew FileStream(path, FileMode::Open, FileAccess::Read, FileShare::Read);
			return serializer->Deserialize(stream);
		}
		catch (Exception^ ex) {
			Log::WriteLine("Exception occured while reading from " + path + " : " + ex->Message);
		}
		finally {
			if (stream)
				delete static_cast<IDisposable^>(stream);
				
			Log::WriteLine("Loaded " + path);
		}
	}

	return nullptr;
}

void Settings::Serialize(String^ path, XmlSerializer^ serializer, Object^ object) {
	if (path != nullptr && serializer != nullptr && object != nullptr) {
		FileStream^ stream = {};

		try {
			stream = gcnew FileStream(path, FileMode::Create, FileAccess::Write, FileShare::None);
			serializer->Serialize(stream, object);
		}
		catch (Exception^ ex) {
			Log::WriteLine("Exception occured while writing to " + path + " : " + ex->Message);
		}
		finally {
			if (stream)
				delete static_cast<IDisposable^>(stream);
			
			Log::WriteLine("Saved " + path);
		}
	}
}

void Settings::Serialize(Control^ c, String^ XmlFileName) {
	XmlTextWriter^ xmlSerializedForm = gcnew XmlTextWriter(XmlFileName, System::Text::Encoding::Default);

	xmlSerializedForm->Formatting = Formatting::Indented;
	xmlSerializedForm->WriteStartDocument();
	xmlSerializedForm->WriteStartElement("Timelapse");

	// enumerate all controls on the form, and serialize them as appropriate
	AddChildControls(xmlSerializedForm, c);

	xmlSerializedForm->WriteEndElement();
	xmlSerializedForm->WriteEndDocument();
	xmlSerializedForm->Flush();
	xmlSerializedForm->Close();

	Log::WriteLine("Saved " + XmlFileName);
}

void Settings::AddChildControls(XmlTextWriter^ xmlSerializedForm, Control^ c) {
	for each(Control^ childCtrl in c->Controls) {
		auto ctrlType = childCtrl->GetType();
		auto ctrlName = childCtrl->Name;

		//TODO: save press state of buttons?
		if (childCtrl->HasChildren || ctrlType == ComboBox::typeid || ctrlType == NumericUpDown::typeid 
			|| ctrlType == CheckBox::typeid || ctrlType == TextBox::typeid || ctrlType == ListBox::typeid) { 
			// serialize this control
			xmlSerializedForm->WriteStartElement("Control");
			xmlSerializedForm->WriteAttributeString("Name", ctrlName);

			if (ctrlType == ComboBox::typeid)
				xmlSerializedForm->WriteAttributeString("SelectedIndex", safe_cast<ComboBox^>(childCtrl)->SelectedIndex.ToString());
			else if (ctrlType == NumericUpDown::typeid)
				xmlSerializedForm->WriteAttributeString("Value", safe_cast<NumericUpDown^>(childCtrl)->Value.ToString());
			else if (ctrlType == CheckBox::typeid)
				xmlSerializedForm->WriteAttributeString("Checked", safe_cast<CheckBox^>(childCtrl)->Checked.ToString());
			else if (ctrlType == TextBox::typeid)
				xmlSerializedForm->WriteAttributeString("Text", safe_cast<TextBox^>(childCtrl)->Text->ToString());
			else if (ctrlType == ListBox::typeid) {
				ListBox^ listBox = safe_cast<ListBox^>(childCtrl);
				int listBoxItemCnt = listBox->Items->Count;

				xmlSerializedForm->WriteAttributeString("ItemCnt", listBoxItemCnt.ToString());

				for (int i = 0; i < listBoxItemCnt; i++) {
					String^ listBoxItemStr = listBox->Items[i]->ToString();
					String^ listBoxItemNmbStr = ctrlName + "Item" + i.ToString();
					xmlSerializedForm->WriteAttributeString(listBoxItemNmbStr, listBoxItemStr);
				}
			}

			// see if this control has any children, and if so, serialize them
			if (childCtrl->HasChildren && ctrlType != NumericUpDown::typeid)
				AddChildControls(xmlSerializedForm, childCtrl);

			xmlSerializedForm->WriteEndElement(); // Control
		}
	}
}

void Settings::Deserialize(Control^ c, String^ XmlFileName) {
	if (File::Exists(XmlFileName)) {
		try {
			XmlDocument^ xmlSerializedForm = gcnew XmlDocument();
			xmlSerializedForm->Load(XmlFileName);

			XmlNode^ topLevel = xmlSerializedForm->ChildNodes[1];
			for each(XmlNode^ n in topLevel->ChildNodes)
				SetControlProperties(safe_cast<Control^>(c), n);
		}
		catch (Exception^ ex) {
			Log::WriteLine("While deserializing \"" + c->Name + "\" the following exception occured: \"" + ex->Message + "\"");
		}
	}
}

void Settings::SetControlProperties(Control^ currentCtrl, XmlNode^ n) {
	try {
		// get the control's name and type
		String^ ctrlName = n->Attributes["Name"]->Value;
		// find the control
		auto ctrl = currentCtrl->Controls->Find(ctrlName, true);

		// the right type too
		if (n->Attributes["SelectedIndex"])
			safe_cast<ComboBox^>(ctrl[0])->SelectedIndex = Convert::ToInt32(n->Attributes["SelectedIndex"]->Value);
		else if (n->Attributes["Value"])
			safe_cast<NumericUpDown^>(ctrl[0])->Value = Convert::ToDecimal(n->Attributes["Value"]->Value);
		else if (n->Attributes["Checked"])
			safe_cast<CheckBox^>(ctrl[0])->Checked = Convert::ToBoolean(n->Attributes["Checked"]->Value);
		else if (n->Attributes["Text"])
			safe_cast<TextBox^>(ctrl[0])->Text = Convert::ToString(n->Attributes["Text"]->Value);
		else if (n->Attributes["ItemCnt"]) {
			int itemCnt = Convert::ToInt32(n->Attributes["ItemCnt"]->Value);
			auto collection = safe_cast<ListBox^>(ctrl[0])->Items;

			if (itemCnt < 1) return;
			for (int i = 0; i < itemCnt; i++) {
				String^ indexedListBoxItemStr = ctrlName + "Item" + i.ToString();
				String^ itemToAddStr = Convert::ToString(n->Attributes[indexedListBoxItemStr]->Value);
				collection->Add(itemToAddStr);
			}
		}

		// if n has any children that are controls, deserialize them as well
		if (n->HasChildNodes && ctrl[0]->HasChildren) {
			XmlNodeList^ xnlControls = n->SelectNodes("Control");
			for each(XmlNode^ n2 in xnlControls)
				SetControlProperties(ctrl[0], n2);
		}
	}
	catch (Exception^ ex) {
		Log::WriteLine("While deserializing \"" + n->Attributes["Name"]->Value + "\" the following exception occured: \"" + ex->Message + "\"");
	}
}

String^ Settings::GetSettingsPath() {
	String^ AppDataFolder = Environment::GetFolderPath(Environment::SpecialFolder::ApplicationData);
	String^ SettingsFilePath = {};

	try {
		String^ TimelapseFolderPath = Path::Combine(AppDataFolder, "Timelapse");
		SettingsFilePath = Path::Combine(TimelapseFolderPath, "Settings.xml");
	}
	catch (Exception^ ex) {
		Log::WriteLine("Exception occured while generating path to settings file" + ex->Message);
	}

	Log::WriteLine("Generated path to Timelapse settings file at" + ": " + SettingsFilePath);
	return SettingsFilePath;
}