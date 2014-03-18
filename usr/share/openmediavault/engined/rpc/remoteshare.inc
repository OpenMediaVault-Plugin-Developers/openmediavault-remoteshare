<?php
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
require_once("openmediavault/object.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceRemoteShare extends OMVRpcServiceAbstract {
    /**
     * Get the RPC service name.
     */
    public function getName() {
        return "remoteshare";
    }

    /**
     * Initialize the RPC service.
     */
    public function initialize() {
        $this->registerMethod("getNfsShareList");
        $this->registerMethod("getNfsShare");
        $this->registerMethod("setNfsShare");
        $this->registerMethod("deleteNfsShare");
        $this->registerMethod("getSmbShareList");
        $this->registerMethod("getSmbShare");
        $this->registerMethod("setSmbShare");
        $this->registerMethod("deleteSmbShare");
    }

    public function getNfsShareList($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "start"     : { "type" : "integer" },
                "limit"     : { '.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].' },
                "sortfield" : { '.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].' },
                "sortdir"   : { '.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].' }
            }
        }');

        // Get the configuration object.
        $xpath = "//services/remoteshare/nfsshares/nfsshare";
        $objects = $xmlConfig->getList($xpath);
        if(is_null($objects))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);

        // Add additional share informations.
        foreach($objects as $objectk => &$objectv) {
            $objectv['sharename'] = gettext("n/a");
            // Get the shared folder configuration object.
            $sfObject = $xmlConfig->get("//system/shares/sharedfolder[uuid='{$objectv['sharedfolderref']}']");
            if(!is_null($sfObject) && !empty($sfObject))
                $objectv['sharename'] = $sfObject['name'];
        }

        // Filter the result.
        return $this->applyFilter($objects, $params['start'], $params['limit'],
            $params['sortfield'], $params['sortdir']);
    }

    public function getNfsShare($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' }
            }
        }');

        // Get the configuration object.
        $xpath = sprintf("//services/remoteshare/nfsshares/nfsshare[uuid='%s']", $params['uuid']);
        $object = $xmlConfig->get($xpath);
        if(is_null($object))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);

        // Modify the result object.
        $object['nfs'] = boolval($object['nfs']);

        return $object;
    }

    public function setNfsShare($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid"            : { '.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].' },
                "server"          : { "type" : "string" },
                "export"          : { "type" : "string" },
                "sharedfolderref" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' },
                "nfs"             : { "type" : "boolean" },
                "extraoptions"    : { "type" : "string" }
            }
        }');

        // Look for duplicates
        if(!empty($params["sharedfolderref"]) && $params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
            if(TRUE === $xmlConfig->exists("//services/remoteshare/nfsshares/nfsshare[sharedfolder='{$params["sharedfolderref"]}']")) {
                throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                    gettext("This share is already in use by another remote NFS share."));
            }
        }

        // Prepare the configuration object.
        $object = array(
            "uuid"            => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ? OMVUtil::uuid() : $params['uuid'],
            "server"          => $params['server'],
            "export"          => $params['export'],
            "sharedfolderref" => $params['sharedfolderref'],
            "nfs"             => array_boolval($params, 'nfs'),
            "extraoptions"    => $params['extraoptions'],
        );

        // Create fstab line
        if($object['nfs'] === true)
            $nfs = "nfs4";
        else
            $nfs = "nfs";

        if(!empty($object['extraoptions']))
            $extraOptions = $object['extraoptions'];
        else
            $extraOptions = "intr";

        $sfPath = OMVRpc::exec("ShareMgmt", "getPath", array("uuid" => $params['sharedfolderref']), $context);

        // <server>:</remote/export></local/directory><nfs-type><options> 0 0
        // server:/usr/local/pub /pub nfs rsize=8192,wsize=8192,timeo=14,intr
        $object['fstab_line'] = $object['server'].":".$object['export']." ".$sfPath." ".$nfs." ".$extraOptions." 0 0";

        // Set the configuration object.
        $success = FALSE;
        if($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
            // Append the configuration object.
            $success = $xmlConfig->set("//services/remoteshare/nfsshares", array("nfsshare" => $object));
        } else {
            // Update the existing configuration object.
            $success = $xmlConfig->replace("//services/remoteshare/nfsshares/nfsshare[uuid='{$params['uuid']}']", $object);
        }
        if(FALSE === $success)
            throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);

        // Notify configuration changes.
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
            OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
            "org.openmediavault.services.remoteshare.nfsshares.nfsshare", $object);

        // Return the configuration object.
        return $object;
    }

    public function deleteNfsShare($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' }
            }
        }');

        // Get the configuration object.
        $xpath = sprintf("//services/remoteshare/nfsshares/nfsshare[uuid='%s']", $params['uuid']);
        $object = $xmlConfig->get($xpath);
        if(is_null($object))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);

        $sfPath = OMVRpc::exec("ShareMgmt", "getPath", array("uuid" => $object['sharedfolderref']), $context);

        $cmd = sprintf("export LANG=C; umount %s", $sfPath);
        $this->exec($cmd);

        // Delete the configuration object.
        $object = $this->deleteConfigObjectByPath($xpath,
            "org.openmediavault.services.remoteshare.nfsshares.nfsshare");

        // Apply the changes immediately (no user interaction is wanted).
        OMVRpc::exec("Config", "applyChanges", array(
            "modules" => array("remoteshare"),
            "force" => TRUE
        ), $context);

        // Return the deleted configuration object.
        return $object;
    }

    public function getSmbShareList($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "start"     : { "type" : "integer" },
                "limit"     : { '.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].' },
                "sortfield" : { '.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].' },
                "sortdir"   : { '.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].' }
            }
        }');

        // Get the configuration object.
        $xpath = "//services/remoteshare/smbshares/smbshare";
        $objects = $xmlConfig->getList($xpath);
        if(is_null($objects))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);

        // Add additional share informations.
        foreach($objects as $objectk => &$objectv) {
            $objectv['sharename'] = gettext("n/a");
            // Get the shared folder configuration object.
            $sfObject = $xmlConfig->get("//system/shares/sharedfolder[uuid='{$objectv['sharedfolderref']}']");
            if(!is_null($sfObject) && !empty($sfObject))
                $objectv['sharename'] = $sfObject['name'];

            if($objectv['guest'] === true)
                $objectv['mountas'] = "guest";
            else
                $objectv['mountas'] = $objectv['username'];
        }

        // Filter the result.
        return $this->applyFilter($objects, $params['start'], $params['limit'],
            $params['sortfield'], $params['sortdir']);
    }

    public function getSmbShare($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' }
            }
        }');

        // Get the configuration object.
        $xpath = sprintf("//services/remoteshare/smbshares/smbshare[uuid='%s']", $params['uuid']);
        $object = $xmlConfig->get($xpath);
        if(is_null($object))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);

        // Modify the result object.
        $object['guest']   = boolval($object['guest']);
        $object['usefile'] = boolval($object['usefile']);

        return $object;
    }

    public function setSmbShare($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid"            : { '.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].' },
                "server"          : { "type" : "string" },
                "smbname"         : { "type" : "string" },
                "sharedfolderref" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' },
                "guest"           : { "type" : "boolean" },
                "username"        : { "type" : "string", "optional" : true },
                "password"        : { "type" : "string", "optional" : true },
                "usefile"         : { "type" : "boolean", "optional" : true }
            }
        }');

        // Look for duplicates
        if(!empty($params["sharedfolderref"]) && $params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
            if(TRUE === $xmlConfig->exists("//services/remoteshare/nfsshares/nfsshare[sharedfolder='{$params["sharedfolderref"]}']")) {
                throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                    gettext("This share is already in use by another remote NFS share."));
            }
            if(TRUE === $xmlConfig->exists("//services/remoteshare/smbshares/smbshare[sharedfolder='{$params["sharedfolderref"]}']")) {
                throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                    gettext("This share is already in use by another remote Samba share."));
            }
        }

        // Prepare the configuration object.
        $object = array(
            "uuid"            => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ? OMVUtil::uuid() : $params['uuid'],
            "server"          => $params['server'],
            "smbname"         => $params['smbname'],
            "sharedfolderref" => $params['sharedfolderref'],
            "guest"           => array_boolval($params, 'guest'),
            "username"        => $params['username'],
            "password"        => $params['password'],
            "usefile"         => array_boolval($params, 'usefile')
        );

        $sfPath = OMVRpc::exec("ShareMgmt", "getPath", array("uuid" => $params['sharedfolderref']), $context);

        // //servername/sharename  /media/windowsshare  cifs  guest,uid=1000,iocharset=utf8  0  0
        // //servername/sharename  /media/windowsshare  cifs  username=msusername,password=mspassword,iocharset=utf8,sec=ntlm  0  0
        // //servername/sharename  /media/windowsshare  cifs  credentials=/home/ubuntuusername/.smbcredentials,iocharset=utf8,sec=ntlm 0 0
        // .smbcredentials
        //   username=msusername
        //   password=mspassword
        if( $object['guest'] === true ) {
            $credentials = "guest";
        } else {
            if( $object['usefile'] === true )
                $credentials = "credentials=/root/.smbcredentials-".$object['sharedfolderref'];
            else
                $credentials = "username=".$object['username'].",password=".$object['password'].",sec=ntlm";
        }
        $object['fstab_line'] = "//".$object['server']."/".$object['smbname']." ".$sfPath." cifs ".$credentials.",iocharset=utf8 0 0";

        // Set the configuration object.
        $success = FALSE;
        if($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
            // Append the configuration object.
            $success = $xmlConfig->set("//services/remoteshare/smbshares", array("smbshare" => $object));
        } else {
            // Update the existing configuration object.
            $success = $xmlConfig->replace("//services/remoteshare/smbshares/smbshare[uuid='{$params['uuid']}']", $object);
        }
        if(FALSE === $success)
            throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);

        // Notify configuration changes.
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
            OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
            "org.openmediavault.services.remoteshare.smbshares.smbshare", $object);

        // Return the configuration object.
        return $object;
    }

    public function deleteSmbShare($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array( "role" => OMV_ROLE_ADMINISTRATOR ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' }
            }
        }');

        // Get the configuration object.
        $xpath = sprintf("//services/remoteshare/smbshares/smbshare[uuid='%s']", $params['uuid']);
        $object = $xmlConfig->get($xpath);
        if(is_null($object))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);

        $sfPath = OMVRpc::exec("ShareMgmt", "getPath", array("uuid" => $object['sharedfolderref']), $context);

        $cmd = sprintf("export LANG=C; umount %s", $sfPath);
        $this->exec($cmd);

        // Delete the configuration object.
        $object = $this->deleteConfigObjectByPath($xpath,
            "org.openmediavault.services.remoteshare.smbshares.smbshare");

        // Apply the changes immediately (no user interaction is wanted).
        OMVRpc::exec("Config", "applyChanges", array(
            "modules" => array("remoteshare"),
            "force" => TRUE
        ), $context);

        // Return the deleted configuration object.
        return $object;
    }
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceRemoteShare());
?>
