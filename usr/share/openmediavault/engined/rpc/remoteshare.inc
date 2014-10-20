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
    const NFSSHARE     = 'nfsshare';
    const SMBSHARE     = 'smbshare';
    const WEBDAVSHARE  = 'webdavshare';
    const GLUSTERSHARE = 'glustershare';
    const SSHSHARE     = 'sshshare';
    const FTPSHARE     = 'ftpshare';

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

    private function getXPathGlusterShares()
    {
        return sprintf( "%s/%ss", $this->getXPath(), self::GLUSTERSHARE );
    }

    private function getXPathGlusterShare()
    {
        return sprintf( "%s/%s", $this->getXPathGlusterShares(), self::GLUSTERSHARE );
    }

    private function getXPathSshShares()
    {
        return sprintf( "%s/%ss", $this->getXPath(), self::SSHSHARE );
    }

    private function getXPathSshShare()
    {
        return sprintf( "%s/%s", $this->getXPathSshShares(), self::SSHSHARE );
    }

    private function getXPathFtpShares()
    {
        return sprintf( "%s/%ss", $this->getXPath(), self::FTPSHARE );
    }

    private function getXPathFtpShare()
    {
        return sprintf( "%s/%s", $this->getXPathFtpShares(), self::FTPSHARE );
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

    private function getEventMessagePathGluster()
    {
        return sprintf( "%s.%ss.%s", $this->getEventMessagePath(), self::GLUSTERSHARE, self::GLUSTERSHARE  );
    }

    private function getEventMessagePathSsh()
    {
        return sprintf( "%s.%ss.%s", $this->getEventMessagePath(), self::SSHSHARE, self::SSHSHARE  );
    }

    private function getEventMessagePathFtp()
    {
        return sprintf( "%s.%ss.%s", $this->getEventMessagePath(), self::FTPSHARE, self::FTPSHARE  );
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

        $this->registerMethod("getGlusterShareList");
        $this->registerMethod("getGlusterShare");
        $this->registerMethod("setGlusterShare");
        $this->registerMethod("deleteGlusterShare");

        $this->registerMethod("getSshShareList");
        $this->registerMethod("getSshShare");
        $this->registerMethod("setSshShare");
        $this->registerMethod("deleteSshShare");

        $this->registerMethod("getFtpShareList");
        $this->registerMethod("getFtpShare");
        $this->registerMethod("setFtpShare");
        $this->registerMethod("deleteFtpShare");
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

    public function setNfsShare($params, $context)
    {
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
        if(!empty($params["sharedfolderref"]))
        {
            // determine if it is the same shared folder as was in use before (when settings are updated)
            $sameshare = FALSE;
            if($params['uuid'] != $GLOBALS['OMV_UUID_UNDEFINED'])
            {
                $xPath = sprintf("%s[uuid='%s']", $this->getXPathNfsShare(), $params['uuid']);
                if (TRUE === $xmlConfig->exists($xPath))
                {
                    $object = $xmlConfig->get($xPath);
                    if ($params["sharedfolderref"] === $object["sharedfolderref"])
                    {
                        $sameshare = TRUE;
                    }
                }
            }

            // we don't need to check if the shared folder was in use before
            if (FALSE === $sameshare)
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
                $xPath = sprintf("%s[sharedfolderref='%s']", $this->getXPathGlusterShare(), $params['sharedfolderref']);
                if (TRUE === $xmlConfig->exists($xPath))
                {
                    throw new OMVException(
                        OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                        gettext("This share is already in use by another remote GlusterFS share.")
                    );
                }
                $xPath = sprintf("%s[sharedfolderref='%s']", $this->getXPathSshShare(), $params['sharedfolderref']);
                if (TRUE === $xmlConfig->exists($xPath))
                {
                    throw new OMVException(
                        OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                        gettext("This share is already in use by another remote sshfs share.")
                    );
                }
                $xPath = sprintf("%s[sharedfolderref='%s']", $this->getXPathFtpShare(), $params['sharedfolderref']);
                if (TRUE === $xmlConfig->exists($xPath))
                {
                    throw new OMVException(
                        OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                        gettext("This share is already in use by another remote FTP share.")
                    );
                }
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
        $object['fstab_line'] = sprintf("%s:%s %s %s %s 0 0", $object['server'], $object['export'], $sfPath, $nfs, $extraOptions);

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
            $xPath = sprintf("%s[uuid='%s']", $this->getXPathNfsShare(), $params['uuid']);
            $success = $xmlConfig->replace($xPath, $object);
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

    public function getSmbShareList($params, $context)
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
        if(!empty($params["sharedfolderref"]))
        {
            // determine if it is the same shared folder as was in use before (when settings are updated)
            $sameshare = FALSE;
            if($params['uuid'] != $GLOBALS['OMV_UUID_UNDEFINED'])
            {
                $xPath = sprintf("%s[uuid='%s']", $this->getXPathSmbShare(), $params['uuid']);
                if (TRUE === $xmlConfig->exists($xPath))
                {
                    $object = $xmlConfig->get($xPath);
                    if ($params["sharedfolderref"] === $object["sharedfolderref"])
                    {
                        $sameshare = TRUE;
                    }
                }
            }

            // we don't need to check if the shared folder was in use before
            if (FALSE === $sameshare)
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
                $xPath = sprintf("%s[sharedfolderref='%s']", $this->getXPathGlusterShare(), $params['sharedfolderref']);
                if (TRUE === $xmlConfig->exists($xPath))
                {
                    throw new OMVException(
                        OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                        gettext("This share is already in use by another remote GlusterFS share.")
                    );
                }
                $xPath = sprintf("%s[sharedfolderref='%s']", $this->getXPathSshShare(), $params['sharedfolderref']);
                if (TRUE === $xmlConfig->exists($xPath))
                {
                    throw new OMVException(
                        OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                        gettext("This share is already in use by another remote sshfs share.")
                    );
                }
                $xPath = sprintf("%s[sharedfolderref='%s']", $this->getXPathFtpShare(), $params['sharedfolderref']);
                if (TRUE === $xmlConfig->exists($xPath))
                {
                    throw new OMVException(
                        OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                        gettext("This share is already in use by another remote FTP share.")
                    );
                }
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
        $smbname = str_replace(" ", "\\\\040", $object['smbname']);
        $object['fstab_line'] = sprintf("//%s/%s %s cifs %s,_netdev,iocharset=utf8 0 0", $object['server'], $smbname, $sfPath, $credentials);

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
            $xPath = sprintf("%s[uuid='%s']", $this->getXPathSmbShare(), $params['uuid']);
            $success = $xmlConfig->replace($xPath, $object);
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
        if(!empty($params["sharedfolderref"]))
        {
            // determine if it is the same shared folder as was in use before (when settings are updated)
            $sameshare = FALSE;
            if($params['uuid'] != $GLOBALS['OMV_UUID_UNDEFINED'])
            {
                $xPath = sprintf("%s[uuid='%s']", $this->getXPathWebdavShare(), $params['uuid']);
                if (TRUE === $xmlConfig->exists($xPath))
                {
                    $object = $xmlConfig->get($xPath);
                    if ($params["sharedfolderref"] === $object["sharedfolderref"])
                    {
                        $sameshare = TRUE;
                    }
                }
            }

            // we don't need to check if the shared folder was in use before
            if (FALSE === $sameshare)
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
                $xPath = sprintf("%s[sharedfolderref='%s']", $this->getXPathGlusterShare(), $params['sharedfolderref']);
                if (TRUE === $xmlConfig->exists($xPath))
                {
                    throw new OMVException(
                        OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                        gettext("This share is already in use by another remote GlusterFS share.")
                    );
                }
                $xPath = sprintf("%s[sharedfolderref='%s']", $this->getXPathSshShare(), $params['sharedfolderref']);
                if (TRUE === $xmlConfig->exists($xPath))
                {
                    throw new OMVException(
                        OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                        gettext("This share is already in use by another remote sshfs share.")
                    );
                }
                $xPath = sprintf("%s[sharedfolderref='%s']", $this->getXPathFtpShare(), $params['sharedfolderref']);
                if (TRUE === $xmlConfig->exists($xPath))
                {
                    throw new OMVException(
                        OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                        gettext("This share is already in use by another remote FTP share.")
                    );
                }
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
        $object['fstab_line'] = sprintf("%s %s davfs %s 0 0", $object['resource'], $sfPath, $extraOptions);

        // Set the configuration object.
        $success = FALSE;
        if ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED'])
        {
            // Append the configuration object.
            $success = $xmlConfig->set($this->getXPathWebdavShares(), array("webdavshare" => $object));
        } else {
            // Update the existing configuration object.
            $xPath = sprintf("%s[uuid='%s']", $this->getXPathWebdavShare(), $params['uuid']);
            $success = $xmlConfig->replace($xPath, $object);
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

    public function getGlusterShareList($params, $context)
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
        $objects = $xmlConfig->getList( $this->getXPathGlusterShare() );
        if (is_null($objects))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPathGlusterShare()
            );
        }

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

    public function getGlusterShare($params, $context)
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
        $xPath = sprintf("%s[uuid='%s']", $this->getXPathGlusterShare(), $params['uuid']);
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

        return $object;
    }

    public function setGlusterShare($params, $context)
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
                "volname"         : { "type" : "string" },
                "sharedfolderref" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' }
            }
        }');

        // Look for duplicates
        if(!empty($params["sharedfolderref"]))
        {
            // determine if it is the same shared folder as was in use before (when settings are updated)
            $sameshare = FALSE;
            if($params['uuid'] != $GLOBALS['OMV_UUID_UNDEFINED'])
            {
                $xPath = sprintf("%s[uuid='%s']", $this->getXPathGlusterShare(), $params['uuid']);
                if (TRUE === $xmlConfig->exists($xPath))
                {
                    $object = $xmlConfig->get($xPath);
                    if ($params["sharedfolderref"] === $object["sharedfolderref"])
                    {
                        $sameshare = TRUE;
                    }
                }
            }

            // we don't need to check if the shared folder was in use before
            if (FALSE === $sameshare)
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
                $xPath = sprintf("%s[sharedfolderref='%s']", $this->getXPathGlusterShare(), $params['sharedfolderref']);
                if (TRUE === $xmlConfig->exists($xPath))
                {
                    throw new OMVException(
                        OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                        gettext("This share is already in use by another remote GlusterFS share.")
                    );
                }
                $xPath = sprintf("%s[sharedfolderref='%s']", $this->getXPathSshShare(), $params['sharedfolderref']);
                if (TRUE === $xmlConfig->exists($xPath))
                {
                    throw new OMVException(
                        OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                        gettext("This share is already in use by another remote sshfs share.")
                    );
                }
                $xPath = sprintf("%s[sharedfolderref='%s']", $this->getXPathFtpShare(), $params['sharedfolderref']);
                if (TRUE === $xmlConfig->exists($xPath))
                {
                    throw new OMVException(
                        OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                        gettext("This share is already in use by another remote FTP share.")
                    );
                }
            }
        }

        // Prepare the configuration object.
        $object = array(
            "uuid"            => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ? OMVUtil::uuid() : $params['uuid'],
            "enable"          => array_boolval($params, 'enable'),
            "server"          => $params['server'],
            "volname"         => $params['smbname'],
            "sharedfolderref" => $params['sharedfolderref']
        );

        $sfPath = OMVRpc::exec("ShareMgmt", "getPath", array("uuid" => $params['sharedfolderref']), $context);

        // HOSTNAME-OR-IPADDRESS:/VOLNAME MOUNTDIR glusterfs defaults,_netdev 0 0
        $object['fstab_line'] = sprintf("%s:/%s %s glusterfs defaults,_netdev 0 0", $object['server'], $object['volname'], $sfPath);

        // Set the configuration object.
        $success = FALSE;
        if ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED'])
        {
            // Append the configuration object.
            $success = $xmlConfig->set($this->getXPathGlusterShares(), array("glustershare" => $object));
        }
        else
        {
            // Update the existing configuration object.
            $xPath = sprintf("%s[uuid='%s']", $this->getXPathGlusterShare(), $params['uuid']);
            $success = $xmlConfig->replace($xPath, $object);
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
            $this->getEventMessagePathGluster(),
            $object
        );

        // Return the configuration object.
        return $object;
    }

    public function deleteGlusterShare($params, $context)
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

    public function getSshShareList($params, $context)
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
        $objects = $xmlConfig->getList( $this->getXPathSshShare() );
        if (is_null($objects))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPathSshShare()
            );
        }

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

    public function getSshShare($params, $context)
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
        $xPath = sprintf("%s[uuid='%s']", $this->getXPathSshShare(), $params['uuid']);
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

        return $object;
    }

    public function setSshShare($params, $context)
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
                "sshname"         : { "type" : "string" },
                "sharedfolderref" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' },
                "username"        : { "type" : "string", "optional" : true },
                "password"        : { "type" : "string", "optional" : true }
            }
        }');

        // Look for duplicates
        if(!empty($params["sharedfolderref"]))
        {
            // determine if it is the same shared folder as was in use before (when settings are updated)
            $sameshare = FALSE;
            if($params['uuid'] != $GLOBALS['OMV_UUID_UNDEFINED'])
            {
                $xPath = sprintf("%s[uuid='%s']", $this->getXPathSshShare(), $params['uuid']);
                if (TRUE === $xmlConfig->exists($xPath))
                {
                    $object = $xmlConfig->get($xPath);
                    if ($params["sharedfolderref"] === $object["sharedfolderref"])
                    {
                        $sameshare = TRUE;
                    }
                }
            }

            // we don't need to check if the shared folder was in use before
            if (FALSE === $sameshare)
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
                $xPath = sprintf("%s[sharedfolderref='%s']", $this->getXPathGlusterShare(), $params['sharedfolderref']);
                if (TRUE === $xmlConfig->exists($xPath))
                {
                    throw new OMVException(
                        OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                        gettext("This share is already in use by another remote GlusterFS share.")
                    );
                }
                $xPath = sprintf("%s[sharedfolderref='%s']", $this->getXPathSshShare(), $params['sharedfolderref']);
                if (TRUE === $xmlConfig->exists($xPath))
                {
                    throw new OMVException(
                        OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                        gettext("This share is already in use by another remote sshfs share.")
                    );
                }
                $xPath = sprintf("%s[sharedfolderref='%s']", $this->getXPathFtpShare(), $params['sharedfolderref']);
                if (TRUE === $xmlConfig->exists($xPath))
                {
                    throw new OMVException(
                        OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                        gettext("This share is already in use by another remote FTP share.")
                    );
                }
            }
        }

        // Prepare the configuration object.
        $object = array(
            "uuid"            => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ? OMVUtil::uuid() : $params['uuid'],
            "enable"          => array_boolval($params, 'enable'),
            "server"          => $params['server'],
            "sshname"         => $params['sshname'],
            "sharedfolderref" => $params['sharedfolderref'],
            "username"        => $params['username'],
            "password"        => $params['password']
        );

        $sfPath = OMVRpc::exec("ShareMgmt", "getPath", array("uuid" => $params['sharedfolderref']), $context);

        // sshfs#USERNAME@REMOTE_HOST:REMOTE_PATH MOUNT_POINT fuse SSHFS_OPTIONS 0 0
        // sshfs#user@server.domain.com:/<your default dir> <mount point> fuse credentials=<path to .credentials>.credentials 0 0
        // .sshcredentials
        //   username=msusername
        //   password=mspassword
        $credentials = sprintf("", );
        $sshname = str_replace(" ", "\\\\040", $object['sshname']);
        $object['fstab_line'] = sprintf(
            "sshfs#%s@%s:%s %s fuse credentials=/root/.sshcredentials-%s,_netdev 0 0",
            $object['username'],
            $object['server'],
            $sshname,
            $sfPath,
            $object['sharedfolderref']
        );

        // Set the configuration object.
        $success = FALSE;
        if ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED'])
        {
            // Append the configuration object.
            $success = $xmlConfig->set($this->getXPathSshShares(), array("sshshare" => $object));
        }
        else
        {
            // Update the existing configuration object.
            $xPath = sprintf("%s[uuid='%s']", $this->getXPathSshShare(), $params['uuid']);
            $success = $xmlConfig->replace($xPath, $object);
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
            $this->getEventMessagePathSsh(),
            $object
        );

        // Return the configuration object.
        return $object;
    }

    public function deleteSshShare($params, $context)
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
        $xPath = sprintf("%s[uuid='%s']", $this->getXPathSshShare(), $params['uuid']);
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
            $this->getEventMessagePathSsh()
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
        $credfile = "/root/.sshcredentials-".$object['sharedfolderref'];
        if (file_exists($credfile))
        {
            unlink($credfile);
        }

        // Return the deleted configuration object.
        return $object;
    }

    public function getFtpShareList($params, $context)
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
        $objects = $xmlConfig->getList( $this->getXPathFtpShare() );
        if (is_null($objects))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPathFtpShare()
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

    public function getFtpShare($params, $context)
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
        $xPath = sprintf("%s[uuid='%s']", $this->getXPathFtpShare(), $params['uuid']);
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

    public function setFtpShare($params, $context)
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
                "sharedfolderref" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' },
                "guest"           : { "type" : "boolean" },
                "username"        : { "type" : "string", "optional" : true },
                "password"        : { "type" : "string", "optional" : true },
                "usefile"         : { "type" : "boolean", "optional" : true }
            }
        }');

        // Look for duplicates
        if(!empty($params["sharedfolderref"]))
        {
            // determine if it is the same shared folder as was in use before (when settings are updated)
            $sameshare = FALSE;
            if($params['uuid'] != $GLOBALS['OMV_UUID_UNDEFINED'])
            {
                $xPath = sprintf("%s[uuid='%s']", $this->getXPathFtpShare(), $params['uuid']);
                if (TRUE === $xmlConfig->exists($xPath))
                {
                    $object = $xmlConfig->get($xPath);
                    if ($params["sharedfolderref"] === $object["sharedfolderref"])
                    {
                        $sameshare = TRUE;
                    }
                }
            }

            // we don't need to check if the shared folder was in use before
            if (FALSE === $sameshare)
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
                $xPath = sprintf("%s[sharedfolderref='%s']", $this->getXPathGlusterShare(), $params['sharedfolderref']);
                if (TRUE === $xmlConfig->exists($xPath))
                {
                    throw new OMVException(
                        OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                        gettext("This share is already in use by another remote GlusterFS share.")
                    );
                }
                $xPath = sprintf("%s[sharedfolderref='%s']", $this->getXPathSshShare(), $params['sharedfolderref']);
                if (TRUE === $xmlConfig->exists($xPath))
                {
                    throw new OMVException(
                        OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                        gettext("This share is already in use by another remote sshfs share.")
                    );
                }
                $xPath = sprintf("%s[sharedfolderref='%s']", $this->getXPathFtpShare(), $params['sharedfolderref']);
                if (TRUE === $xmlConfig->exists($xPath))
                {
                    throw new OMVException(
                        OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                        gettext("This share is already in use by another remote FTP share.")
                    );
                }
            }
        }

        // Prepare the configuration object.
        $object = array(
            "uuid"            => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ? OMVUtil::uuid() : $params['uuid'],
            "enable"          => array_boolval($params, 'enable'),
            "server"          => $params['server'],
            "sharedfolderref" => $params['sharedfolderref'],
            "guest"           => array_boolval($params, 'guest'),
            "username"        => $params['username'],
            "password"        => $params['password'],
            "usefile"         => array_boolval($params, 'usefile')
        );

        $sfPath = OMVRpc::exec("ShareMgmt", "getPath", array("uuid" => $params['sharedfolderref']), $context);

        // curlftpfs#ftp.host.com /mnt/host fuse rw,uid=500,user,noauto 0 0
        // curlftpfs#ftp://user:pass@IP /media/FTP fuse rw,uid=1000,gid=1000,umask=002,user,suid,allow_other,exec,auto,utf8,_netdev  0   0
        // /root/.netrc
        // machine address_of_ftp
        // login ftp_un
        // password ftp_pw

        if ( $object['guest'] === true )
        {
            $object['fstab_line'] = sprintf("curlftpfs#%s %s fuse rw,_netdev 0 0", $object['server'], $sfPath);
        }
        else
        {
            if( $object['usefile'] === true )
            {
                $object['fstab_line'] = sprintf("curlftpfs#%s %s fuse rw,_netdev 0 0", $object['server'], $sfPath);
            }
            else
            {
                $object['fstab_line'] = sprintf("curlftpfs#ftp://%s:%s@%s %s fuse rw,_netdev 0 0", $object['username'], $object['password'], $object['server'], $sfPath);
            }
        }

        // Set the configuration object.
        $success = FALSE;
        if ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED'])
        {
            // Append the configuration object.
            $success = $xmlConfig->set($this->getXPathFtpShares(), array("ftpshare" => $object));
        }
        else
        {
            // Update the existing configuration object.
            $xPath = sprintf("%s[uuid='%s']", $this->getXPathFtpShare(), $params['uuid']);
            $success = $xmlConfig->replace($xPath, $object);
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
            $this->getEventMessagePathFtp(),
            $object
        );

        // Return the configuration object.
        return $object;
    }

    public function deleteFtpShare($params, $context)
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
        $xPath = sprintf("%s[uuid='%s']", $this->getXPathFtpShare(), $params['uuid']);
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
            $this->getEventMessagePathFtp()
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
        $credfile = "/root/.ftpcredentials-".$object['sharedfolderref'];
        if (file_exists($credfile))
        {
            unlink($credfile);
        }

        // Return the deleted configuration object.
        return $object;
    }
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceRemoteShare());
