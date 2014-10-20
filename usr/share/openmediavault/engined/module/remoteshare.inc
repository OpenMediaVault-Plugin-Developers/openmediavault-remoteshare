<?php
/**
 * Copyright (C) 2013-2014 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
require_once("openmediavault/module.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");

class OMVModuleRemoteShare extends OMVModuleServiceAbstract implements OMVINotifyListener
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


    public function applyConfig()
    {
        global $xmlConfig;

        $config = $xmlConfig->get( $this->getXPath() );

        if (is_null($config))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPath()
            );
        }

        $cmd = "omv-mkconf " . $this->getPluginName() . " 2>&1";

        if (0 !== $this->exec($cmd, $output))
        {
            throw new OMVException(
                OMVErrorMsg::E_EXEC_FAILED,
                $cmd,
                implode("\n", $output)
            );
        }
    }

    public function bindListeners(OMVNotifyDispatcher $dispatcher)
    {
        $dispatcher->addListener(
            OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
            $this->getEventMessagePathNfs(),
            array($this, "setDirty")
        );
        $dispatcher->addListener(
            OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
            $this->getEventMessagePathSmb(),
            array($this, "setDirty")
        );
        $dispatcher->addListener(
            OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
            $this->getEventMessagePathWebdav(),
            array($this, "setDirty")
        );
        $dispatcher->addListener(
            OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
            $this->getEventMessagePathGluster(),
            array($this, "setDirty")
        );
        $dispatcher->addListener(
            OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
            $this->getEventMessagePathSsh(),
            array($this, "setDirty")
        );
        $dispatcher->addListener(
            OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
            $this->getEventMessagePathFtp(),
            array($this, "setDirty")
        );        
        $dispatcher->addListener(
            OMV_NOTIFY_MODIFY,
            "org.openmediavault.system.shares.sharedfolder",
            array($this, "onSharedFolder")
        );
    }
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMVModuleRemoteShare());
