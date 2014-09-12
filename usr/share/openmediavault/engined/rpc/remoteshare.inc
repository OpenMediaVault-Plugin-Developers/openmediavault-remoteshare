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

class OMVRpcServiceRemoteShare extends OMVRpcServiceAbstract
{
    const NFSSHARE = 'nfsshare';
    const SMBSHARE = 'smbshare';
    const WEBDAVSHARE = 'webdavshare';

    public function getName()
    {
        return "remoteshare";
    }

    private function getPluginName()
    {
        return strtolower( $this->getName() );
    }

    private function getXPath()
    {
        return sprintf( "//services/%s", $this->getPluginName() );
    }

    private function getXPathNfsShares()
    {
        return sprintf( "%s/%ss", $this->getXPath(), self::NFSSHARE );
    }

    private function getXPathNfsShare()
    {
        return sprintf( "%s/%s", $this->getXPathNfsShares(), self::NFSSHARE );
    }

    private function getXPathSmbShares()
    {
        return sprintf( "%s/%ss", $this->getXPath(), self::SMBSHARE );
    }

    private function getXPathSmbShare()
    {
        return sprintf( "%s/%s", $this->getXPathSmbShares(), self::SMBSHARE );
    }

    private function getXPathWebdavShares()
    {
        return sprintf( "%s/%ss", $this->getXPath(), self::WEBDAVSHARE );
    }

    private function getXPathWebdavShare()
    {
        return sprintf( "%s/%s", $this->getXPathWebdavShares(), self::WEBDAVSHARE );
    }

    private function getEventMessagePath()
    {
        return sprintf( "org.openmediavault.services.%s", $this->getPluginName() );
    }

    private function getEventMessagePathNfs()
    {
        return sprintf( "%s.%ss.%s", $this->getEventMessagePath(), self::NFSSHARE, self::NFSSHARE );
    }

    private function getEventMessagePathSmb()
    {
        return sprintf( "%s.%ss.%s", $this->getEventMessagePath(), self::SMBSHARE, self::SMBSHARE  );
    }

    private function getEventMessagePathWebdav()
    {
        return sprintf( "%s.%ss.%s", $this->getEventMessagePath(), self::WEBDAVSHARE, self::WEBDAVSHARE  );
    }

    public function initialize()
    {
        $this->registerMethod("getNfsShareList");
        $this->registerMethod("getNfsShare");
        $this->registerMethod("setNfsShare");
        $this->registerMethod("deleteNfsShare");
        $this->registerMethod("getSmbShareList");
        $this->registerMethod("getSmbShare");
        $this->registerMethod("setSmbShare");
        $this->registerMethod("deleteSmbShare");
        $this->registerMethod("getWebdavShareList");
        $this->registerMethod("getWebdavShare");
        $this->registerMethod("setWebdavShare");
        $this->registerMethod("deleteWebdavShare");
    }

    public function getNfsShareList($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

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
        $objects = $xmlConfig->getList( $this->getXPathNfsShare() );
        if (is_null($objects))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPathNfsShare()
            );
        }

        // Add additional share informations.
        foreach($objects as $objectk => &$objectv)
        {
            $objectv['sharename'] = gettext("n/a");
            // Get the shared folder configuration object.
            $sfObject = $xmlConfig->get("//system/shares/sharedfolder[uuid='{$objectv['sharedfolderref']}']");
            if (!is_null($sfObject) && !empty($sfObject))
            {
                $objectv['sharename'] = $sfObject['name'];
            }
        }

        // Filter the result.
        return $this->applyFilter($objects, $params['start'], $params['limit'],
            $params['sortfield'], $params['sortdir']);
    }

    public function getNfsShare($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' }
            }
        }');

        // Get the configuration object.
        $xPath = sprintf("%s[uuid='%s']", $this->getXPathNfsShare(), $params['uuid']);
        $object = $xmlConfig->get($xPath);
        if (is_null($object))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $xPath
            );
        }
        // Modify the result object.
        $object['enable'] = boolval($object['enable']);
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
                "enable"          : { "type" : "boolean" },
                "server"          : { "type" : "string" },
                "export"          : { "type" : "string" },
                "sharedfolderref" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' },
                "nfs"             : { "type" : "boolean" },
                "extraoptions"    : { "type" : "string" }
            }
        }');

        // Look for duplicates
        if(!empty($params["sharedfolderref"]) && $params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED'])
        {
            $xPath = sprintf("%s[sharedfolderref='%s']", $this->getXPathNfsShare(), $params['sharedfolderref']);
            if (TRUE === $xmlConfig->exists($xPath))
            {
                throw new OMVException(
                    OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                    gettext("This share is already in use by another remote NFS share.")
                );
            }
            $xPath = sprintf("%s[sharedfolderref='%s']", $this->getXPathSmbShare(), $params['sharedfolderref']);
            if (TRUE === $xmlConfig->exists($xPath))
            {
                throw new OMVException(
                    OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                    gettext("This share is already in use by another remote Samba share.")
                );
            }
            $xPath = sprintf("%s[sharedfolderref='%s']", $this->getXPathWebdavShare(), $params['sharedfolderref']);
            if (TRUE === $xmlConfig->exists($xPath))
            {
                throw new OMVException(
                    OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                    gettext("This share is already in use by another remote WebDAV share.")
                );
            }
        }

        // Prepare the configuration object.
        $object = array(
            "uuid"            => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ? OMVUtil::uuid() : $params['uuid'],
            "enable"          => array_boolval($params, 'enable'),
            "server"          => $params['server'],
            "export"          => $params['export'],
            "sharedfolderref" => $params['sharedfolderref'],
            "nfs"             => array_boolval($params, 'nfs'),
            "extraoptions"    => $params['extraoptions'],
        );

        // Create fstab line
        if ($object['nfs'] === true)
            $nfs = "nfs4";
        else
            $nfs = "nfs";

        if (!empty($object['extraoptions']))
        {
            $extraOptions = $object['extraoptions'];
        }
        else
        {
            $extraOptions = "intr";

        }
        $extrasOptions = "_netdev," . $extraOptions;

        $sfPath = OMVRpc::exec("ShareMgmt", "getPath", array("uuid" => $params['sharedfolderref']), $context);

        // <server>:</remote/export></local/directory><nfs-type><options> 0 0
        // server:/usr/local/pub /pub nfs rsize=8192,wsize=8192,timeo=14,intr
        $object['fstab_line'] = $object['server'].":".$object['export']." ".$sfPath." ".$nfs." ".$extraOptions." 0 0";

        // Set the configuration object.
        $success = FALSE;
        if ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED'])
        {
            // Append the configuration object.
            $success = $xmlConfig->set($this->getXPathNfsShares(), array("nfsshare" => $object));
        }
        else
        {
            // Update the existing configuration object.
            $xPath = sprintf("%s[uuid='%s']", $this->getXPathNfsShare(), $object);
            $success = $xmlConfig->replace($xPath);
        }
        if(FALSE === $success)
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED
            );
        }

        // Notify configuration changes.
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(
            ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ? OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
            $this->getEventMessagePathNfs(),
            $object);

        // Return the configuration object.
        return $object;
    }

    public function deleteNfsShare($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' }
            }
        }');

        // Get the configuration object.
        $xPath = sprintf("%s[uuid='%s']", $this->getXPathNfsShare(), $params['uuid']);
        $object = $xmlConfig->get($xPath);
        if (is_null($object))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $xPath
            );
        }
        $sfPath = OMVRpc::exec("ShareMgmt", "getPath", array("uuid" => $object['sharedfolderref']), $context);

        $cmd = sprintf("export LANG=C; umount %s", $sfPath);
        $this->exec($cmd);

        // Delete the configuration object.
        $object = $this->deleteConfigObjectByPath(
            $xPath,
            $this->getEventMessagePathNfs()
        );

        // Apply the changes immediately (no user interaction is wanted).
        OMVRpc::exec(
            "Config",
            "applyChanges",
            array(
                "modules" => array("remoteshare"),
                "force" => TRUE
            ),
            $context
        );

        // Return the deleted configuration object.
        return $object;
    }

    public function getSmbShareList($params, $context) {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

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
        $objects = $xmlConfig->getList( $this->getXPathSmbShare() );
        if (is_null($objects))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPathSmbShare()
            );
        }

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

    public function getSmbShare($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' }
            }
        }');

        // Get the configuration object.
        $xPath = sprintf("%s[uuid='%s']", $this->getXPathSmbShare(), $params['uuid']);
        $object = $xmlConfig->get($xPath);
        if (is_null($object))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $xPath
            );
        }
        // Modify the result object.
        $object['enable']  = boolval($object['enable']);
        $object['guest']   = boolval($object['guest']);
        $object['usefile'] = boolval($object['usefile']);

        return $object;
    }

    public function setSmbShare($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid"            : { '.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].' },
                "enable"          : { "type" : "boolean" },
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
        if (!empty($params["sharedfolderref"]) && $params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED'])
        {
            $xPath = sprintf("%s[sharedfolderref='%s']", $this->getXPathNfsShare(), $params['sharedfolderref']);
            if (TRUE === $xmlConfig->exists($xPath))
            {
                throw new OMVException(
                    OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                    gettext("This share is already in use by another remote NFS share.")
                );
            }
            $xPath = sprintf("%s[sharedfolderref='%s']", $this->getXPathSmbShare(), $params['sharedfolderref']);
            if (TRUE === $xmlConfig->exists($xPath))
            {
                throw new OMVException(
                    OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                    gettext("This share is already in use by another remote Samba share.")
                );
            }
            $xPath = sprintf("%s[sharedfolderref='%s']", $this->getXPathWebdavShare(), $params['sharedfolderref']);
            if (TRUE === $xmlConfig->exists($xPath))
            {
                throw new OMVException(
                    OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                    gettext("This share is already in use by another remote WebDAV share.")
                );
            }
        }

        // Prepare the configuration object.
        $object = array(
            "uuid"            => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ? OMVUtil::uuid() : $params['uuid'],
            "enable"          => array_boolval($params, 'enable'),
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
        if ( $object['guest'] === true )
        {
            $credentials = "guest";
        }
        else
        {
            if( $object['usefile'] === true )
            {
                $credentials = "credentials=/root/.smbcredentials-".$object['sharedfolderref'];
            }
            else
            {
                $credentials = "username=".$object['username'].",password=".$object['password'].",sec=ntlm";
            }
        }
        $object['fstab_line'] = "//".$object['server']."/".$object['smbname']." ".$sfPath." cifs ".$credentials.",_netdev,iocharset=utf8 0 0";

        // Set the configuration object.
        $success = FALSE;
        if ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED'])
        {
            // Append the configuration object.
            $success = $xmlConfig->set($this->getXPathSmbShares(), array("smbshare" => $object));
        }
        else
        {
            // Update the existing configuration object.
            $xPath = sprintf("%s[uuid='%s']", $this->getXPathSmbShares(), $object);
            $success = $xmlConfig->replace($xPath);
        }
        if (FALSE === $success)
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED
            );
        }

        // Notify configuration changes.
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(
            ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ? OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
            $this->getEventMessagePathSmb(),
            $object
        );

        // Return the configuration object.
        return $object;
    }

    public function deleteSmbShare($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' }
            }
        }');

        // Get the configuration object.
        $xPath = sprintf("%s[uuid='%s']", $this->getXPathSmbShare(), $params['uuid']);
        $object = $xmlConfig->get($xPath);
        if (is_null($object))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $xPath
            );
        }
        $sfPath = OMVRpc::exec("ShareMgmt", "getPath", array("uuid" => $object['sharedfolderref']), $context);

        // unmount share
        $cmd = sprintf("export LANG=C; umount %s", $sfPath);
        $this->exec($cmd);

        // Delete the configuration object.
        $object = $this->deleteConfigObjectByPath(
            $xPath,
            $this->getEventMessagePathSmb()
        );

        // Apply the changes immediately (no user interaction is wanted).
        OMVRpc::exec(
            "Config",
            "applyChanges",
            array(
                "modules" => array("remoteshare"),
                "force" => TRUE
            ),
            $context
        );

        // delete credential file
        $credfile = "/root/.smbcredentials-".$object['sharedfolderref'];
        if (file_exists($credfile))
        {
            unlink($credfile);
        }

        // Return the deleted configuration object.
        return $object;
    }

    public function getWebdavShareList($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

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
        $objects = $xmlConfig->getList( $this->getXPathWebdavShare() );
        if (is_null($objects))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPathWebdavShare()
            );
        }

        // Add additional share informations.
        foreach($objects as $objectk => &$objectv)
        {
            $objectv['sharename'] = gettext("n/a");
            // Get the shared folder configuration object.
            $sfObject = $xmlConfig->get("//system/shares/sharedfolder[uuid='{$objectv['sharedfolderref']}']");
            if (!is_null($sfObject) && !empty($sfObject))
            {
                $objectv['sharename'] = $sfObject['name'];
            }
        }

        // Filter the result.
        return $this->applyFilter($objects, $params['start'], $params['limit'],
            $params['sortfield'], $params['sortdir']);
    }

    private $davfs_defaults = array(
        'use_locks'    => 0,
        'gui_optimize' => 1,
        'if_match_bug' => 1,
        'cache_size'   => 1,
        'table_size'   => 4096,
        'delay_upload' => 1,
   );

    public function getWebdavShare($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' }
            }
        }');

        // Get the configuration object.
        $xPath = sprintf("%s[uuid='%s']", $this->getXPathWebdavShare(), $params['uuid']);
        $object = $xmlConfig->get($xPath);
        if (is_null($object))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $xPpath
            );
        }

        // Modify the result object.
        $object['enable'] = boolval($object['enable']);

        preg_match_all("/([^::]+)::([^;]+)/", $object['davfs_opts'], $davfs_opts);
        $davfs_opts = array_combine($davfs_opts[1], $davfs_opts[2]);

        foreach($this->davfs_defaults as $optk => $optv)
        {
            $object[$optk] = (empty($davfs_opts[$optk])) ? $optv : $davfs_opts[$optk];
        }

        return $object;
    }

    public function setWebdavShare($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid"            : { '.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].' },
                "enable"          : { "type" : "boolean" },
                "resource"        : { "type" : "string" },
                "sharedfolderref" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' },
                "username"        : { "type" : "string", "optional" : true },
                "password"        : { "type" : "string", "optional" : true },
                "extraoptions"    : { "type" : "string" },
                "use_locks"       : { "type" : "boolean" },
                "gui_optimize"    : { "type" : "boolean" },
                "if_match_bug"    : { "type" : "boolean" },
                "cache_size"      : { "type" : "integer", "minimum" : 1, "maximum" : 65535 },
                "table_size"      : { "type" : "integer", "minimum" : 1, "maximum" : 65535 },
                "delay_upload"    : { "type" : "integer", "minimum" : 0, "maximum" : 65535 }
            }
        }');

        // Look for duplicates
        if (!empty($params["sharedfolderref"]) && $params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED'])
        {
            $xPath = sprintf("%s[sharedfolderref='%s']", $this->getXPathNfsShare(), $params['sharedfolderref']);
            if (TRUE === $xmlConfig->exists($xPath))
            {
                throw new OMVException(
                    OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                    gettext("This share is already in use by another remote NFS share.")
                );
            }
            $xPath = sprintf("%s[sharedfolderref='%s']", $this->getXPathSmbShare(), $params['sharedfolderref']);
            if (TRUE === $xmlConfig->exists($xPath))
            {
                throw new OMVException(
                    OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                    gettext("This share is already in use by another remote Samba share.")
                );
            }
            $xPath = sprintf("%s[sharedfolderref='%s']", $this->getXPathWebdavShare(), $params['sharedfolderref']);
            if (TRUE === $xmlConfig->exists($xPath))
            {
                throw new OMVException(
                    OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                    gettext("This share is already in use by another remote WebDAV share.")
                );
            }
        }

        // Prepare the configuration object.
        $object = array(
            "uuid"            => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ? OMVUtil::uuid() : $params['uuid'],
            "enable"          => array_boolval($params, 'enable'),
            "resource"        => $params['resource'],
            "sharedfolderref" => $params['sharedfolderref'],
            "username"        => $params['username'],
            "password"        => $params['password'],
            "extraoptions"    => $params['extraoptions'],
        );

        foreach($this->davfs_defaults as $optk => $optv)
        {
            $optarr[] = $optk."::".(empty($params[$optk]) ? $optv : $params[$optk]);
        }
        $object['davfs_opts'] = implode(';', $optarr);

        if(!empty($object['extraoptions']))
        {
            $extraOptions = $object['extraoptions'];
        }
        else
        {
            $extraOptions = "_netdev";
        }

        $sfPath = OMVRpc::exec("ShareMgmt", "getPath", array("uuid" => $params['sharedfolderref']), $context);
        rtrim($sfPath, "/");

        // <resourceurl></local/directory><davfs><options> 0 0
        // https://server/path /pub davfs _netdev,user,rw,noauto  0  0
        $object['fstab_line'] = $object['resource']." ".$sfPath." davfs ".$extraOptions." 0 0";

        // Set the configuration object.
        $success = FALSE;
        if ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED'])
        {
            // Append the configuration object.
            $success = $xmlConfig->set($this->getXPathWebdavShares(), array("webdavshare" => $object));
        } else {
            // Update the existing configuration object.
            $xPath = sprintf("%s[uuid='%s']", $this->getXPathWebdavShares(), $object);
            $success = $xmlConfig->replace($xPath);
        }
        if (FALSE === $success)
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED
            );
        }

        // Notify configuration changes.
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(
            ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ? OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
            $this->getEventMessagePathWebdav(),
            $object
        );

        // Return the configuration object.
        return $object;
    }

    public function deleteWebdavShare($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' }
            }
        }');

        // Get the configuration object.
        $xPath = sprintf("%s[uuid='%s']", $this->getXPathWebdavShare(), $params['uuid']);
        $object = $xmlConfig->get($xPath);
        if (is_null($object))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $xPath
            );
        }
        $sfPath = OMVRpc::exec("ShareMgmt", "getPath", array("uuid" => $object['sharedfolderref']), $context);

        // unmount share
        $cmd = sprintf("export LANG=C; umount %s", $sfPath);
        $this->exec($cmd);

        // Delete the configuration object.
        $object = $this->deleteConfigObjectByPath(
            $xPath,
            $this->getEventMessagePathWebdav()
        );

        // Apply the changes immediately (no user interaction is wanted).
        OMVRpc::exec(
            "Config",
            "applyChanges",
            array(
                "modules" => array("remoteshare"),
                "force" => TRUE
            ),
            $context
        );

        // Return the deleted configuration object.
        return $object;
    }
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceRemoteShare());
