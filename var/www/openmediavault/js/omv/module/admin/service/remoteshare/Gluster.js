/**
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
 * @copyright Copyright (c) 2013-2015 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/grid/Panel.js")
// require("js/omv/workspace/window/Form.js")
// require("js/omv/workspace/window/plugin/ConfigObject.js")
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/form/field/SharedFolderComboBox.js")

Ext.define("OMV.module.admin.service.remoteshare.GlusterShare", {
    extend : "OMV.workspace.window.Form",
    uses   : [
        "OMV.form.field.SharedFolderComboBox",
        "OMV.workspace.window.plugin.ConfigObject",
        "OMV.form.plugin.LinkedFields"
    ],

    rpcService   : "remoteshare",
    rpcGetMethod : "getGlusterShare",
    rpcSetMethod : "setGlusterShare",

    plugins      : [{
        ptype : "configobject"
    }],

    width        : 550,

    getFormItems : function() {
        var me = this;
        return [{
            xtype      : "checkbox",
            name       : "enable",
            fieldLabel : _("Enable"),
            checked    : true
        },{
            xtype      : "textfield",
            name       : "server",
            fieldLabel : _("Server"),
            allowBlank : false,
            plugins    : [{
                ptype : "fieldinfo",
                text  : _("Hostname, IP address, or fully qualified domain name of the server.")
            }]
        },{
            xtype      : "textfield",
            name       : "volname",
            fieldLabel : _("Volume Name"),
            allowBlank : false
        },{
            xtype      : "sharedfoldercombo",
            name       : "sharedfolderref",
            fieldLabel : _("Shared Folder"),
            plugins    : [{
                ptype : "fieldinfo",
                text  : _("Gluster share will be mounted as this shared folder for use in other plugins.")
            }]
        }];
    }
});

Ext.define("OMV.module.admin.service.remoteshare.GlusterShares", {
    extend   : "OMV.workspace.grid.Panel",
    requires : [
        "OMV.Rpc",
        "OMV.data.Store",
        "OMV.data.Model",
        "OMV.data.proxy.Rpc"
    ],
    uses     : [
        "OMV.module.admin.service.remoteshare.GlusterShare"
    ],

    hidePagingToolbar : false,
    stateful          : true,
    stateId           : "1649057b-b1c0-1c48-a4c1-2b8d1fe52d7b",
    columns           : [{
        xtype     : "booleaniconcolumn",
        text      : _("Enabled"),
        sortable  : true,
        dataIndex : "enable",
        stateId   : "enable",
        align     : "center",
        width     : 80,
        resizable : false,
        trueIcon  : "switch_on.png",
        falseIcon : "switch_off.png"
    },{
        text      : _("Server"),
        sortable  : true,
        dataIndex : "server",
        stateId   : "server"
    },{
        text      : _("Volume Name"),
        sortable  : true,
        dataIndex : "volname",
        stateId   : "volname"
    },{
        text      : _("Shared Folder"),
        sortable  : true,
        dataIndex : "sharename",
        stateId   : "sharename"
    }],

    initComponent : function() {
        var me = this;
        Ext.apply(me, {
            store : Ext.create("OMV.data.Store", {
                autoLoad : true,
                model    : OMV.data.Model.createImplicit({
                    idProperty  : "uuid",
                    fields      : [
                        { name : "uuid", type: "string" },
                        { name : "enable", type: "boolean" },
                        { name : "server", type: "string" },
                        { name : "volname", type: "string" },
                        { name : "sharename", type: "string" }
                    ]
                }),
                proxy : {
                    type    : "rpc",
                    rpcData : {
                        service : "remoteshare",
                        method  : "getGlusterShareList"
                    }
                }
            })
        });
        me.callParent(arguments);
    },

    onAddButton : function() {
        var me = this;
        Ext.create("OMV.module.admin.service.remoteshare.GlusterShare", {
            title     : _("Add share"),
            uuid      : OMV.UUID_UNDEFINED,
            listeners : {
                scope  : me,
                submit : function() {
                    this.doReload();
                }
            }
        }).show();
    },

    onEditButton : function() {
        var me = this;
        var record = me.getSelected();
        Ext.create("OMV.module.admin.service.remoteshare.GlusterShare", {
            title     : _("Edit share"),
            uuid      : record.get("uuid"),
            listeners : {
                scope  : me,
                submit : function() {
                    this.doReload();
                }
            }
        }).show();
    },

    doDeletion : function(record) {
        var me = this;
        OMV.Rpc.request({
            scope    : me,
            callback : me.onDeletion,
            rpcData : {
                service : "remoteshare",
                method  : "deleteGlusterShare",
                params  : {
                    uuid : record.get("uuid")
                }
            }
        });
    }
});

OMV.WorkspaceManager.registerPanel({
    id        : "glustershares",
    path      : "/service/remoteshare",
    text      : _("GlusterFS"),
    position  : 40,
    className : "OMV.module.admin.service.remoteshare.GlusterShares"
});
