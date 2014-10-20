/**
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

Ext.define("OMV.module.admin.service.remoteshare.SshShare", {
    extend : "OMV.workspace.window.Form",
    uses   : [
        "OMV.form.field.SharedFolderComboBox",
        "OMV.workspace.window.plugin.ConfigObject",
        "OMV.form.plugin.LinkedFields"
    ],

    rpcService   : "remoteshare",
    rpcGetMethod : "getSshShare",
    rpcSetMethod : "setSshShare",

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
            fieldLabel : _("Remote Host"),
            allowBlank : false,
            plugins    : [{
                ptype : "fieldinfo",
                text  : _("Hostname, IP address, or fully qualified domain name of the server.")
            }]
        },{
            xtype      : "textfield",
            name       : "sshname",
            fieldLabel : _("Remote Path"),
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
            xtype      : "textfield",
            name       : "username",
            fieldLabel : _("Username"),
            allowBlank : false
        },{
            xtype      : "textfield",
            name       : "password",
            fieldLabel : _("Password"),
            allowBlank : false
        }];
    }
});

Ext.define("OMV.module.admin.service.remoteshare.SshShares", {
    extend   : "OMV.workspace.grid.Panel",
    requires : [
        "OMV.Rpc",
        "OMV.data.Store",
        "OMV.data.Model",
        "OMV.data.proxy.Rpc"
    ],
    uses     : [
        "OMV.module.admin.service.remoteshare.SshShare"
    ],

    hidePagingToolbar : false,
    stateful          : true,
    stateId           : "1649057b-b1c0-1c48-a4c1-8c8d1fe52c7b",
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
        text      : _("Remote Host"),
        sortable  : true,
        dataIndex : "server",
        stateId   : "server"
    },{
        text      : _("Remote Path"),
        sortable  : true,
        dataIndex : "sshname",
        stateId   : "sshname"
    },{
        text      : _("Shared Folder"),
        sortable  : true,
        dataIndex : "sharename",
        stateId   : "sharename"
    },{
        text      : _("Mount as"),
        sortable  : true,
        dataIndex : "username",
        stateId   : "username"
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
                        { name : "sshname", type: "string" },
                        { name : "sharename", type: "string" },
                        { name : "username", type: "string" }
                    ]
                }),
                proxy : {
                    type    : "rpc",
                    rpcData : {
                        service : "remoteshare",
                        method  : "getSshShareList"
                    }
                }
            })
        });
        me.callParent(arguments);
    },

    onAddButton : function() {
        var me = this;
        Ext.create("OMV.module.admin.service.remoteshare.SshShare", {
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
        Ext.create("OMV.module.admin.service.remoteshare.SshShare", {
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
                method  : "deleteSshShare",
                params  : {
                    uuid : record.get("uuid")
                }
            }
        });
    }
});

OMV.WorkspaceManager.registerPanel({
    id        : "sshshares",
    path      : "/service/remoteshare",
    text      : _("SSHFS"),
    position  : 50,
    className : "OMV.module.admin.service.remoteshare.SshShares"
});
