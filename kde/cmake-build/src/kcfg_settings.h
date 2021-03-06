// This file is generated by kconfig_compiler from sflphone-client-kde.kcfg.
// All changes you do to this file will be lost.
#ifndef CONFIGURATIONSKELETONBASE_H
#define CONFIGURATIONSKELETONBASE_H

#include <kglobal.h>
#include <kconfigskeleton.h>
#include <kdebug.h>

class ConfigurationSkeletonBase : public KConfigSkeleton
{
  public:
    class EnumInterface
    {
      public:
      enum type { ALSA, PulseAudio, COUNT };
    };

    ConfigurationSkeletonBase( );
    ~ConfigurationSkeletonBase();

    /**
      Set Defines the port that will be used for SIP communication.
    */
    void setSIPPort( int v )
    {
      if (v <  1025 )
      {
        kDebug() << "setSIPPort: value " << v << " is less than the minimum value of  1025 ";
        v =  1025 ;
      }

      if (v >  65536 )
      {
        kDebug() << "setSIPPort: value " << v << " is greater than the maximum value of  65536 ";
        v =  65536 ;
      }

      if (!isImmutable( QString::fromLatin1( "SIPPort" ) ))
        mSIPPort = v;
    }

    /**
      Get Defines the port that will be used for SIP communication.
    */
    int sIPPort() const
    {
      return mSIPPort;
    }

    /**
      Set Defines whether sflphone should keep a history of calls.
    */
    void setEnableHistory( bool v )
    {
      if (!isImmutable( QString::fromLatin1( "enableHistory" ) ))
        mEnableHistory = v;
    }

    /**
      Get Defines whether sflphone should keep a history of calls.
    */
    bool enableHistory() const
    {
      return mEnableHistory;
    }

    /**
      Set Defines the number of days the history has to be kept.
    */
    void setHistoryMax( int v )
    {
      if (v <  1 )
      {
        kDebug() << "setHistoryMax: value " << v << " is less than the minimum value of  1 ";
        v =  1 ;
      }

      if (v >  99 )
      {
        kDebug() << "setHistoryMax: value " << v << " is greater than the maximum value of  99 ";
        v =  99 ;
      }

      if (!isImmutable( QString::fromLatin1( "historyMax" ) ))
        mHistoryMax = v;
    }

    /**
      Get Defines the number of days the history has to be kept.
    */
    int historyMax() const
    {
      return mHistoryMax;
    }

    /**
      Set Defines whether user should be notified when receiving a call.
    */
    void setNotifOnCalls( bool v )
    {
      if (!isImmutable( QString::fromLatin1( "notifOnCalls" ) ))
        mNotifOnCalls = v;
    }

    /**
      Get Defines whether user should be notified when receiving a call.
    */
    bool notifOnCalls() const
    {
      return mNotifOnCalls;
    }

    /**
      Set Defines whether user should be notified when receiving a message.
    */
    void setNotifOnMessages( bool v )
    {
      if (!isImmutable( QString::fromLatin1( "notifOnMessages" ) ))
        mNotifOnMessages = v;
    }

    /**
      Get Defines whether user should be notified when receiving a message.
    */
    bool notifOnMessages() const
    {
      return mNotifOnMessages;
    }

    /**
      Set Defines whether the main window should be displayed on start.
    */
    void setDisplayOnStart( bool v )
    {
      if (!isImmutable( QString::fromLatin1( "displayOnStart" ) ))
        mDisplayOnStart = v;
    }

    /**
      Get Defines whether the main window should be displayed on start.
    */
    bool displayOnStart() const
    {
      return mDisplayOnStart;
    }

    /**
      Set Defines whether the main window should be displayed when receiving a message.
    */
    void setDisplayOnCalls( bool v )
    {
      if (!isImmutable( QString::fromLatin1( "displayOnCalls" ) ))
        mDisplayOnCalls = v;
    }

    /**
      Get Defines whether the main window should be displayed when receiving a message.
    */
    bool displayOnCalls() const
    {
      return mDisplayOnCalls;
    }

    /**
      Set Defines the Stun server to use.
    */
    void setInterface( int v )
    {
      if (!isImmutable( QString::fromLatin1( "interface" ) ))
        mInterface = v;
    }

    /**
      Get Defines the Stun server to use.
    */
    int interface() const
    {
      return mInterface;
    }

    /**
      Set Defines whether ringtones are enabled.
    */
    void setEnableRingtones( bool v )
    {
      if (!isImmutable( QString::fromLatin1( "enableRingtones" ) ))
        mEnableRingtones = v;
    }

    /**
      Get Defines whether ringtones are enabled.
    */
    bool enableRingtones() const
    {
      return mEnableRingtones;
    }

    /**
      Set Defines which ringtone is used.
    */
    void setRingtone( const QString & v )
    {
      if (!isImmutable( QString::fromLatin1( "ringtone" ) ))
        mRingtone = v;
    }

    /**
      Get Defines which ringtone is used.
    */
    QString ringtone() const
    {
      return mRingtone;
    }

    /**
      Set Defines which ALSA plugin to use.
    */
    void setAlsaPlugin( const QString & v )
    {
      if (!isImmutable( QString::fromLatin1( "alsaPlugin" ) ))
        mAlsaPlugin = v;
    }

    /**
      Get Defines which ALSA plugin to use.
    */
    QString alsaPlugin() const
    {
      return mAlsaPlugin;
    }

    /**
      Set Defines which ALSA Input device to use.
    */
    void setAlsaInputDevice( int v )
    {
      if (!isImmutable( QString::fromLatin1( "alsaInputDevice" ) ))
        mAlsaInputDevice = v;
    }

    /**
      Get Defines which ALSA Input device to use.
    */
    int alsaInputDevice() const
    {
      return mAlsaInputDevice;
    }

    /**
      Set Defines which ALSA Output device to use.
    */
    void setAlsaOutputDevice( int v )
    {
      if (!isImmutable( QString::fromLatin1( "alsaOutputDevice" ) ))
        mAlsaOutputDevice = v;
    }

    /**
      Get Defines which ALSA Output device to use.
    */
    int alsaOutputDevice() const
    {
      return mAlsaOutputDevice;
    }

    /**
      Set Defines whether pulse audio can mute other applications during a call.
    */
    void setPulseAudioVolumeAlter( bool v )
    {
      if (!isImmutable( QString::fromLatin1( "pulseAudioVolumeAlter" ) ))
        mPulseAudioVolumeAlter = v;
    }

    /**
      Get Defines whether pulse audio can mute other applications during a call.
    */
    bool pulseAudioVolumeAlter() const
    {
      return mPulseAudioVolumeAlter;
    }

    /**
      Set Defines whether the search in KDE Address Book is enabled
    */
    void setEnableAddressBook( bool v )
    {
      if (!isImmutable( QString::fromLatin1( "enableAddressBook" ) ))
        mEnableAddressBook = v;
    }

    /**
      Get Defines whether the search in KDE Address Book is enabled
    */
    bool enableAddressBook() const
    {
      return mEnableAddressBook;
    }

    /**
      Set Defines the max number of contacts to display during a search in address book.
    */
    void setMaxResults( int v )
    {
      if (!isImmutable( QString::fromLatin1( "maxResults" ) ))
        mMaxResults = v;
    }

    /**
      Get Defines the max number of contacts to display during a search in address book.
    */
    int maxResults() const
    {
      return mMaxResults;
    }

    /**
      Set Defines whether to display contacts photos.
    */
    void setDisplayPhoto( bool v )
    {
      if (!isImmutable( QString::fromLatin1( "displayPhoto" ) ))
        mDisplayPhoto = v;
    }

    /**
      Get Defines whether to display contacts photos.
    */
    bool displayPhoto() const
    {
      return mDisplayPhoto;
    }

    /**
      Set Defines whether to display professionnal phone numbers.
    */
    void setBusiness( bool v )
    {
      if (!isImmutable( QString::fromLatin1( "business" ) ))
        mBusiness = v;
    }

    /**
      Get Defines whether to display professionnal phone numbers.
    */
    bool business() const
    {
      return mBusiness;
    }

    /**
      Set Defines whether to display mobile phone numbers.
    */
    void setMobile( bool v )
    {
      if (!isImmutable( QString::fromLatin1( "mobile" ) ))
        mMobile = v;
    }

    /**
      Get Defines whether to display mobile phone numbers.
    */
    bool mobile() const
    {
      return mMobile;
    }

    /**
      Set Defines whether to display personnal phone numbers.
    */
    void setHome( bool v )
    {
      if (!isImmutable( QString::fromLatin1( "home" ) ))
        mHome = v;
    }

    /**
      Get Defines whether to display personnal phone numbers.
    */
    bool home() const
    {
      return mHome;
    }

    /**
      Set Defines the destination directory for call recordings.
    */
    void setDestinationFolder( const QString & v )
    {
      if (!isImmutable( QString::fromLatin1( "destinationFolder" ) ))
        mDestinationFolder = v;
    }

    /**
      Get Defines the destination directory for call recordings.
    */
    QString destinationFolder() const
    {
      return mDestinationFolder;
    }

    /**
      Set Defines whether to enable hooks for SIP accounts.
    */
    void setEnableHooksSIP( bool v )
    {
      if (!isImmutable( QString::fromLatin1( "enableHooksSIP" ) ))
        mEnableHooksSIP = v;
    }

    /**
      Get Defines whether to enable hooks for SIP accounts.
    */
    bool enableHooksSIP() const
    {
      return mEnableHooksSIP;
    }

    /**
      Set Defines whether to enable hooks for IAX accounts.
    */
    void setEnableHooksIAX( bool v )
    {
      if (!isImmutable( QString::fromLatin1( "enableHooksIAX" ) ))
        mEnableHooksIAX = v;
    }

    /**
      Get Defines whether to enable hooks for IAX accounts.
    */
    bool enableHooksIAX() const
    {
      return mEnableHooksIAX;
    }

    /**
      Set Defines which header to catch for SIP accounts hooks.
    */
    void setHooksSIPHeader( const QString & v )
    {
      if (!isImmutable( QString::fromLatin1( "hooksSIPHeader" ) ))
        mHooksSIPHeader = v;
    }

    /**
      Get Defines which header to catch for SIP accounts hooks.
    */
    QString hooksSIPHeader() const
    {
      return mHooksSIPHeader;
    }

    /**
      Set Defines which command to execute.
    */
    void setHooksCommand( const QString & v )
    {
      if (!isImmutable( QString::fromLatin1( "hooksCommand" ) ))
        mHooksCommand = v;
    }

    /**
      Get Defines which command to execute.
    */
    QString hooksCommand() const
    {
      return mHooksCommand;
    }

    /**
      Set Defines whether to add a prefix for outgoing calls.
    */
    void setAddPrefix( bool v )
    {
      if (!isImmutable( QString::fromLatin1( "addPrefix" ) ))
        mAddPrefix = v;
    }

    /**
      Get Defines whether to add a prefix for outgoing calls.
    */
    bool addPrefix() const
    {
      return mAddPrefix;
    }

    /**
      Set Defines the prefix to add.
    */
    void setPrepend( const QString & v )
    {
      if (!isImmutable( QString::fromLatin1( "prepend" ) ))
        mPrepend = v;
    }

    /**
      Get Defines the prefix to add.
    */
    QString prepend() const
    {
      return mPrepend;
    }

  protected:

    // main
    int mSIPPort;
    bool mEnableHistory;
    int mHistoryMax;
    bool mNotifOnCalls;
    bool mNotifOnMessages;
    bool mDisplayOnStart;
    bool mDisplayOnCalls;
    int mInterface;
    bool mEnableRingtones;
    QString mRingtone;
    QString mAlsaPlugin;
    int mAlsaInputDevice;
    int mAlsaOutputDevice;
    bool mPulseAudioVolumeAlter;
    bool mEnableAddressBook;
    int mMaxResults;
    bool mDisplayPhoto;
    bool mBusiness;
    bool mMobile;
    bool mHome;
    QString mDestinationFolder;
    bool mEnableHooksSIP;
    bool mEnableHooksIAX;
    QString mHooksSIPHeader;
    QString mHooksCommand;
    bool mAddPrefix;
    QString mPrepend;

  private:
};

#endif

