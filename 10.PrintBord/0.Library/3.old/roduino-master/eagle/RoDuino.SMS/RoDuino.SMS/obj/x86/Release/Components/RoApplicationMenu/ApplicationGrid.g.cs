﻿#pragma checksum "..\..\..\..\..\Components\RoApplicationMenu\ApplicationGrid.xaml" "{406ea660-64cf-4c82-b6f0-42d48172a799}" "985121A649FF9A8F2C48B4721780D7B1"
//------------------------------------------------------------------------------
// <auto-generated>
//     This code was generated by a tool.
//     Runtime Version:4.0.30319.239
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
// </auto-generated>
//------------------------------------------------------------------------------

using RoDuino.SMS.Components;
using System;
using System.Diagnostics;
using System.Windows;
using System.Windows.Automation;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Media.Effects;
using System.Windows.Media.Imaging;
using System.Windows.Media.Media3D;
using System.Windows.Media.TextFormatting;
using System.Windows.Navigation;
using System.Windows.Shapes;


namespace RoDuino.SMS.Components.RoApplicationMenu {
    
    
    /// <summary>
    /// ApplicationGrid
    /// </summary>
    public partial class ApplicationGrid : System.Windows.Controls.Grid, System.Windows.Markup.IComponentConnector {
        
        
        #line 8 "..\..\..\..\..\Components\RoApplicationMenu\ApplicationGrid.xaml"
        internal System.Windows.Controls.Border borderbackground;
        
        #line default
        #line hidden
        
        
        #line 10 "..\..\..\..\..\Components\RoApplicationMenu\ApplicationGrid.xaml"
        internal System.Windows.Controls.Grid borderBrush;
        
        #line default
        #line hidden
        
        
        #line 11 "..\..\..\..\..\Components\RoApplicationMenu\ApplicationGrid.xaml"
        internal RoDuino.SMS.Components.RoImageButton btnCloseApp;
        
        #line default
        #line hidden
        
        private bool _contentLoaded;
        
        /// <summary>
        /// InitializeComponent
        /// </summary>
        [System.Diagnostics.DebuggerNonUserCodeAttribute()]
        public void InitializeComponent() {
            if (_contentLoaded) {
                return;
            }
            _contentLoaded = true;
            System.Uri resourceLocater = new System.Uri("/RoDuino.SMS;component/components/roapplicationmenu/applicationgrid.xaml", System.UriKind.Relative);
            
            #line 1 "..\..\..\..\..\Components\RoApplicationMenu\ApplicationGrid.xaml"
            System.Windows.Application.LoadComponent(this, resourceLocater);
            
            #line default
            #line hidden
        }
        
        [System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
        internal System.Delegate _CreateDelegate(System.Type delegateType, string handler) {
            return System.Delegate.CreateDelegate(delegateType, this, handler);
        }
        
        [System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [System.ComponentModel.EditorBrowsableAttribute(System.ComponentModel.EditorBrowsableState.Never)]
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Design", "CA1033:InterfaceMethodsShouldBeCallableByChildTypes")]
        void System.Windows.Markup.IComponentConnector.Connect(int connectionId, object target) {
            switch (connectionId)
            {
            case 1:
            this.borderbackground = ((System.Windows.Controls.Border)(target));
            return;
            case 2:
            this.borderBrush = ((System.Windows.Controls.Grid)(target));
            return;
            case 3:
            this.btnCloseApp = ((RoDuino.SMS.Components.RoImageButton)(target));
            return;
            }
            this._contentLoaded = true;
        }
    }
}

