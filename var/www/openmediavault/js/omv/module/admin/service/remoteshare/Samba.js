/**
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
 * @copyright Copyright (c) 2013-2014 OpenMediaVault Plugin Developers
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

Ext.define("OMV.module.admin.service.remoteshare.SmbShare", {
    extend : "OMV.workspace.window.Form",
    uses   : [
        "OMV.form.field.SharedFolderComboBox",
        "OMV.workspace.window.plugin.ConfigObject",
        "OMV.form.plugin.LinkedFields"
    ],

    rpcService   : "remoteshare",
    rpcGetMethod : "getSmbShare",
    rpcSetMethod : "setSmbShare",

    plugins      : [{
        ptype : "configobject"
    },{
        ptype        : "linkedfields",
        correlations : [{
            conditions : [{
                name  : "guest",
                value : false
            }],
            name       : [
                "username",
                "password",
                "usefile"
            ],
            properties : "show"
        },{
            conditions : [{
                name  : "guest",
                value : true
            }],
            name       : [
                "username",
                "password"
            ],
            properties : "allowBlank"
        }]
    }],

    width        : 550,

    getFormItems : function() {
        var me = this;
        return [{
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
            name       : "smbname",
            fieldLabel : _("Share Name"),
            allowBlank : false
        },{
            xtype      : "sharedfoldercombo",
            name       : "sharedfolderref",
            fieldLabel : _("Shared Folder"),
            plugins    : [{
                ptype : "fieldinfo",
                text  : _("Samba share will be mounted as this shared folder for use in other plugins.")
            }]
        },{
            xtype      : "checkbox",
            name       : "guest",
            fieldLabel : _("Guest"),
            checked    : false,
            plugins    : [{
                ptype : "fieldinfo",
                text  : _("If checked, share is mounted as guest.")
            }]
        },{
            xtype      : "textfield",
            name       : "username",
            fieldLabel : _("Username"),
            allowBlank : false
        },{
            xtype      : "textfield",
            name       : "password",
            fieldLabel : _("Password"),
            allowBlank : false
        },{
            xtype      : "checkbox",
            name       : "usefile",
            fieldLabel : _("Use File"),
            checked    : false,
            plugins    : [{
                ptype : "fieldinfo",
                text  : _("If checked, username and password are stored in a credential file instead of in fstab.")
            }]
        }];
    }
});

Ext.define("OMV.module.admin.service.remoteshare.SmbShares", {
    extend   : "OMV.workspace.grid.Panel",
    requires : [
        "OMV.Rpc",
        "OMV.data.Store",
        "OMV.data.Model",
        "OMV.data.proxy.Rpc"
    ],
    uses     : [
        "OMV.module.admin.service.remoteshare.SmbShare"
    ],

    hidePagingToolbar : false,
    stateful          : true,
    stateId           : "1649057b-b1c0-1c48-a4c1-8c8d1fe52d7b",
    columns           : [{
        text      : _("Server"),
        sortable  : true,
        dataIndex : "server",
        stateId   : "server"
    },{
        text      : _("Share Name"),
        sortable  : true,
        dataIndex : "smbname",
        stateId   : "smbname"
    },{
        text      : _("Shared Folder"),
        sortable  : true,
        dataIndex : "sharename",
        stateId   : "sharename"
    },{
        text      : _("Mount as"),
        sortable  : true,
        dataIndex : "mountas",
        stateId   : "mountas"
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
                        { name : "server", type: "string" },
                        { name : "smbname", type: "string" },
                        { name : "sharename", type: "string" },
                        { name : "mountas", type: "string" }
                    ]
                }),
                proxy : {
                    type    : "rpc",
                    rpcData : {
                        service : "remoteshare",
                        method  : "getSmbShareList"
                    }
                }
            })
        });
        me.callParent(arguments);
    },

    onAddButton : function() {
        var me = this;
        Ext.create("OMV.module.admin.service.remoteshare.SmbShare", {
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
        Ext.create("OMV.module.admin.service.remoteshare.SmbShare", {
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
                method  : "deleteSmbShare",
                params  : {
                    uuid : record.get("uuid")
                }
            }
        });
    }
});

OMV.WorkspaceManager.registerPanel({
    id        : "smbshares",
    path      : "/service/remoteshare",
    text      : _("Samba"),
    position  : 20,
    className : "OMV.module.admin.service.remoteshare.SmbShares"
});
