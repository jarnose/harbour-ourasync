<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="fi_FI">
<context>
    <name>AboutPage</name>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="16"/>
        <source>About Oura Sync</source>
        <translation>Tietoja: Oura Sync</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="22"/>
        <source>Oura Sync reads an Oura ring directly over Bluetooth LE and shows what it recorded. It is an independent project: it is not made, endorsed or supported by Oura Health, and it is not a medical device.</source>
        <translation>Oura Sync lukee Oura-sormuksen suoraan Bluetooth LE:n yli ja näyttää, mitä se on tallentanut. Tämä on riippumaton projekti: Oura Health ei ole tehnyt, hyväksynyt eikä tue sitä, eikä se ole lääkinnällinen laite.</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="27"/>
        <source>What you see, and where it comes from</source>
        <translation>Mitä näet ja mistä se tulee</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="35"/>
        <source>Measured by the ring: sleep stages and the sleep window, heart rate, heart rate variability, skin temperature, blood oxygen, breathing rate while asleep, and how hard you moved, minute by minute.

Estimated by this app: the sleep score (from published sleep research), the temperature change against your own usual, active calories, and the hourly heart rate bands.

Not available: Oura&apos;s own Sleep, Readiness and Activity scores, and step counts. Oura computes those on the phone with closed software, so a ring read directly does not carry them.</source>
        <translation>Sormuksen mittaamaa: unen vaiheet ja unijakso, syke, sykevaihtelu, ihon lämpötila, veren happikyllästeisyys, hengitystiheys unen aikana sekä liikkeen rasittavuus minuutti minuutilta.

Tämän sovelluksen arvioimaa: unipisteet (julkaistun unitutkimuksen pohjalta), lämpötilan poikkeama omasta tavallisesta tasostasi, aktiivisesti kulutetut kalorit ja tuntikohtaiset sykealueet.

Ei saatavilla: Ouran omat uni-, valmiusaste- ja aktiivisuuspisteet sekä askeleet. Oura laskee ne puhelimessa suljetulla ohjelmistolla, joten suoraan luettu sormus ei sisällä niitä.</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="46"/>
        <source>Privacy</source>
        <translation>Yksityisyys</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="54"/>
        <source>The app has no network permission. The history is kept in a database on this phone and the ring&apos;s key in the Sailfish secrets vault.</source>
        <translation>Sovelluksella ei ole verkkolupaa. Historia on tietokannassa tässä puhelimessa ja sormuksen avain Sailfishin salaisuusholvissa.</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="58"/>
        <source>Credits</source>
        <translation>Tekijät ja kiitokset</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="66"/>
        <source>The ring&apos;s protocol was reverse engineered by the open_oura project, and the sleep-score method and several decoders come from open_health, both by Thomas Marchand (MIT licence). The dashboards follow the ideas of Cracked Oura. The protocol details were checked against version 7.27.0 of Oura&apos;s Android app.</source>
        <translation>Sormuksen protokollan on selvittänyt open_oura-projekti, ja unipisteiden menetelmä sekä useat dekooderit tulevat open_health-projektista; molemmat ovat Thomas Marchandin (MIT-lisenssi). Näkymät seuraavat Cracked Ouran ideoita. Protokollan yksityiskohdat tarkistettiin Ouran Android-sovelluksen version 7.27.0 koodista.</translation>
    </message>
</context>
<context>
    <name>AppController</name>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="231"/>
        <source>Could not open the local database: %1</source>
        <translation>Paikallista tietokantaa ei voitu avata: %1</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="277"/>
        <source>Another operation is still running.</source>
        <translation>Toinen toiminto on vielä käynnissä.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="294"/>
        <source>Done</source>
        <translation>Valmis</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="433"/>
        <location filename="../src/controller/appcontroller.cpp" line="460"/>
        <source>The secrets vault did not answer.</source>
        <translation>Salaisuusholvi ei vastannut.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="487"/>
        <source>Connecting to the ring…</source>
        <translation>Yhdistetään sormukseen…</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="496"/>
        <source>The ring&apos;s key is not in the secrets vault (%1). Set the ring up again, or enter its key.</source>
        <translation>Sormuksen avainta ei löydy salaisuusholvista (%1). Ota sormus käyttöön uudelleen tai anna sen avain.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="497"/>
        <source>not found</source>
        <translation>ei löytynyt</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="499"/>
        <location filename="../src/controller/appcontroller.cpp" line="658"/>
        <location filename="../src/controller/appcontroller.cpp" line="699"/>
        <source>Authenticating…</source>
        <translation>Todennetaan…</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="505"/>
        <source>The ring has been factory reset and has no key. Set it up again.</source>
        <translation>Sormus on palautettu tehdasasetuksiin eikä sillä ole avainta. Ota se käyttöön uudelleen.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="507"/>
        <source>The ring rejected the stored key. Another app has probably set the ring up since; a factory reset and a new set-up are needed.</source>
        <translation>Sormus hylkäsi tallennetun avaimen. Jokin toinen sovellus on luultavasti ottanut sormuksen käyttöön sen jälkeen; tarvitaan tehdasasetusten palautus ja uusi käyttöönotto.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="509"/>
        <source>Authentication failed: %1</source>
        <translation>Todennus epäonnistui: %1</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="529"/>
        <source>Preparing the ring…</source>
        <translation>Valmistellaan sormusta…</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="532"/>
        <source>Could not prepare the ring: %1</source>
        <translation>Sormuksen valmistelu epäonnistui: %1</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="538"/>
        <source>Could not set the ring&apos;s clock: %1</source>
        <translation>Sormuksen kelloa ei voitu asettaa: %1</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="550"/>
        <source>Downloading history…</source>
        <translation>Ladataan historiaa…</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="558"/>
        <source>Downloading history: %1 events</source>
        <translation>Ladataan historiaa: %1 tapahtumaa</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="564"/>
        <source>History download stopped: %1</source>
        <translation>Historian lataus pysähtyi: %1</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="605"/>
        <source>No ring is set up yet.</source>
        <translation>Sormusta ei ole vielä otettu käyttöön.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="608"/>
        <source>Syncing…</source>
        <translation>Synkronoidaan…</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="615"/>
        <source>Bluetooth does not know this ring any more. Set it up again.</source>
        <translation>Bluetooth ei enää tunne tätä sormusta. Ota se käyttöön uudelleen.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="633"/>
        <location filename="../src/controller/appcontroller.cpp" line="730"/>
        <source>Pairing…</source>
        <translation>Pariutetaan…</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="641"/>
        <location filename="../src/controller/appcontroller.cpp" line="721"/>
        <source>Oura ring</source>
        <translation>Oura-sormus</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="663"/>
        <source>Installing the ring&apos;s key…</source>
        <translation>Asennetaan sormuksen avainta…</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="667"/>
        <source>Could not read the system&apos;s random source.</source>
        <translation>Järjestelmän satunnaislähdettä ei voitu lukea.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="676"/>
        <location filename="../src/controller/appcontroller.cpp" line="725"/>
        <source>Could not store the key in the secrets vault: %1</source>
        <translation>Avainta ei voitu tallentaa salaisuusholviin: %1</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="686"/>
        <source>The ring would not take a new key: %1

A ring that is still set up with the Oura app must first be factory reset.</source>
        <translation>Sormus ei ottanut uutta avainta: %1

Sormus, joka on yhä otettu käyttöön Oura-sovelluksella, täytyy ensin palauttaa tehdasasetuksiin.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="715"/>
        <source>A key is 32 hexadecimal characters (16 bytes).</source>
        <translation>Avain on 32 heksadesimaalimerkkiä (16 tavua).</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="718"/>
        <source>Storing the key…</source>
        <translation>Tallennetaan avainta…</translation>
    </message>
</context>
<context>
    <name>CoverPage</name>
    <message>
        <location filename="../qml/cover/CoverPage.qml" line="55"/>
        <source>Slept %1</source>
        <translation>Nukuttu %1</translation>
    </message>
    <message>
        <location filename="../qml/cover/CoverPage.qml" line="72"/>
        <source>No history yet</source>
        <translation>Ei vielä historiaa</translation>
    </message>
    <message>
        <location filename="../qml/cover/CoverPage.qml" line="72"/>
        <source>No ring set up</source>
        <translation>Sormusta ei otettu käyttöön</translation>
    </message>
</context>
<context>
    <name>DayPage</name>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="36"/>
        <source>Active, estimated</source>
        <translation>Aktiivinen, arvio</translation>
    </message>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="42"/>
        <source>Moderate or harder</source>
        <translation>Kohtalaista tai rankempaa</translation>
    </message>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="48"/>
        <source>Light</source>
        <translation>Kevyt</translation>
    </message>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="54"/>
        <source>Lowest heart rate</source>
        <translation>Alin syke</translation>
    </message>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="60"/>
        <source>Highest heart rate</source>
        <translation>Korkein syke</translation>
    </message>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="66"/>
        <source>Average effort</source>
        <translation>Keskimääräinen rasitus</translation>
    </message>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="70"/>
        <source>Heart rate by hour (bpm)</source>
        <translation>Syke tunneittain (bpm)</translation>
    </message>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="78"/>
        <source>Effort by hour (MET)</source>
        <translation>Rasitus tunneittain (MET)</translation>
    </message>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="93"/>
        <source>The ring reports how hard you moved, minute by minute, as a MET value (1 is sitting still). It does not report steps. Active calories are an estimate: each minute above 1 MET counts (MET − 1) × your weight ÷ 60, with the weight from Settings. Heart rate bars show the 5th to 95th percentile of the hour, with a tick at the mean.</source>
        <translation>Sormus kertoo liikkeen rasittavuuden minuutti minuutilta MET-arvona (1 on paikallaan istumista). Se ei kerro askelia. Aktiiviset kalorit ovat arvio: jokainen 1 METin ylittävä minuutti lasketaan (MET − 1) × painosi ÷ 60, paino Asetuksista. Sykepalkit näyttävät tunnin 5.–95. persentiilin ja keskiarvon pienenä viivana.</translation>
    </message>
</context>
<context>
    <name>DaysPage</name>
    <message>
        <location filename="../qml/pages/DaysPage.qml" line="24"/>
        <source>Activity</source>
        <translation>Aktiivisuus</translation>
    </message>
    <message>
        <location filename="../qml/pages/DaysPage.qml" line="25"/>
        <source>%1 days</source>
        <translation>%1 päivää</translation>
    </message>
    <message>
        <location filename="../qml/pages/DaysPage.qml" line="52"/>
        <source>Heart rate %1–%2 bpm</source>
        <translation>Syke %1–%2 bpm</translation>
    </message>
    <message>
        <location filename="../qml/pages/DaysPage.qml" line="53"/>
        <source>No heart rate</source>
        <translation>Ei sykettä</translation>
    </message>
    <message>
        <location filename="../qml/pages/DaysPage.qml" line="54"/>
        <source>%1 min active</source>
        <translation>%1 min aktiivinen</translation>
    </message>
    <message>
        <location filename="../qml/pages/DaysPage.qml" line="65"/>
        <source>%1 kcal</source>
        <translation>%1 kcal</translation>
    </message>
    <message>
        <location filename="../qml/pages/DaysPage.qml" line="71"/>
        <source>No activity yet</source>
        <translation>Ei vielä aktiivisuutta</translation>
    </message>
    <message>
        <location filename="../qml/pages/DaysPage.qml" line="72"/>
        <source>Sync the ring after wearing it for a day</source>
        <translation>Synkronoi sormus päivän käytön jälkeen</translation>
    </message>
</context>
<context>
    <name>DiagnosticsPage</name>
    <message>
        <location filename="../qml/pages/DiagnosticsPage.qml" line="18"/>
        <source>Diagnostics</source>
        <translation>Diagnostiikka</translation>
    </message>
    <message>
        <location filename="../qml/pages/DiagnosticsPage.qml" line="27"/>
        <source>Stored events by type</source>
        <translation>Tallennetut tapahtumat tyypeittäin</translation>
    </message>
</context>
<context>
    <name>Hypnogram</name>
    <message>
        <location filename="../qml/components/Hypnogram.qml" line="51"/>
        <source>Awake</source>
        <translation>Hereillä</translation>
    </message>
    <message>
        <location filename="../qml/components/Hypnogram.qml" line="51"/>
        <source>REM</source>
        <translation>REM</translation>
    </message>
    <message>
        <location filename="../qml/components/Hypnogram.qml" line="51"/>
        <source>Light</source>
        <translation>Kevyt</translation>
    </message>
    <message>
        <location filename="../qml/components/Hypnogram.qml" line="51"/>
        <source>Deep</source>
        <translation>Syvä</translation>
    </message>
</context>
<context>
    <name>KeyPage</name>
    <message>
        <location filename="../qml/pages/KeyPage.qml" line="37"/>
        <source>Ring is ready</source>
        <translation>Sormus on valmis</translation>
    </message>
    <message>
        <location filename="../qml/pages/KeyPage.qml" line="37"/>
        <source>Ring key</source>
        <translation>Sormuksen avain</translation>
    </message>
    <message>
        <location filename="../qml/pages/KeyPage.qml" line="46"/>
        <source>The ring is set up and its first history is on its way into the app.</source>
        <translation>Sormus on otettu käyttöön ja sen ensimmäinen historia on latautumassa sovellukseen.</translation>
    </message>
    <message>
        <location filename="../qml/pages/KeyPage.qml" line="55"/>
        <source>Your ring is protected by a key that only this phone knows. It is stored in the secure vault. If you lose this phone or its data you will need the key to read the ring again; without it the only way back is a factory reset, which erases the ring&apos;s stored history. Save a copy somewhere safe, such as a password manager.</source>
        <translation>Sormustasi suojaa avain, jonka vain tämä puhelin tuntee. Se on tallessa suojatussa holvissa. Jos menetät tämän puhelimen tai sen tiedot, tarvitset avaimen sormuksen lukemiseen uudelleen; ilman sitä ainoa tie takaisin on tehdasasetusten palautus, joka poistaa sormuksen tallennetun historian. Tallenna kopio turvalliseen paikkaan, esimerkiksi salasanamanageriin.</translation>
    </message>
    <message>
        <location filename="../qml/pages/KeyPage.qml" line="64"/>
        <source>Show key</source>
        <translation>Näytä avain</translation>
    </message>
    <message>
        <location filename="../qml/pages/KeyPage.qml" line="80"/>
        <source>The vault did not return a key.</source>
        <translation>Holvi ei palauttanut avainta.</translation>
    </message>
    <message>
        <location filename="../qml/pages/KeyPage.qml" line="86"/>
        <source>Copy</source>
        <translation>Kopioi</translation>
    </message>
    <message>
        <location filename="../qml/pages/KeyPage.qml" line="97"/>
        <source>Anyone with this key who can pair with the ring can read it. Clear the clipboard after pasting it somewhere safe.</source>
        <translation>Jokainen, jolla on tämä avain ja joka voi pariutua sormuksen kanssa, voi lukea sen. Tyhjennä leikepöytä, kun olet liittänyt avaimen turvalliseen paikkaan.</translation>
    </message>
    <message>
        <location filename="../qml/pages/KeyPage.qml" line="104"/>
        <source>Done</source>
        <translation>Valmis</translation>
    </message>
</context>
<context>
    <name>LineChart</name>
    <message>
        <location filename="../qml/components/LineChart.qml" line="79"/>
        <source>No data</source>
        <translation>Ei tietoja</translation>
    </message>
</context>
<context>
    <name>MainPage</name>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="32"/>
        <source>Not synced yet</source>
        <translation>Ei vielä synkronoitu</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="34"/>
        <source>Synced just now</source>
        <translation>Synkronoitu juuri nyt</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="34"/>
        <source>Synced %1 ago</source>
        <translation>Synkronoitu %1 sitten</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="44"/>
        <source>About</source>
        <translation>Tietoja</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="48"/>
        <source>Settings</source>
        <translation>Asetukset</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="53"/>
        <location filename="../qml/pages/MainPage.qml" line="134"/>
        <source>Set up ring</source>
        <translation>Ota sormus käyttöön</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="59"/>
        <source>Sync now</source>
        <translation>Synkronoi nyt</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="70"/>
        <source>Oura Sync</source>
        <translation>Oura Sync</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="121"/>
        <source>Your ring, without the cloud</source>
        <translation>Sormuksesi ilman pilveä</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="128"/>
        <source>Oura Sync reads your ring over Bluetooth and shows its sleep, heart rate, HRV, temperature and activity. There is no account, no subscription and no network access: the data stays on this phone.</source>
        <translation>Oura Sync lukee sormuksesi Bluetoothilla ja näyttää sen unen, sykkeen, sykevaihtelun, lämpötilan ja aktiivisuuden. Ei tiliä, ei tilausta eikä verkkoyhteyttä: tiedot pysyvät tässä puhelimessa.</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="147"/>
        <source>Reading the history…</source>
        <translation>Luetaan historiaa…</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="148"/>
        <source>No history yet. Pull down and choose Sync now, with the ring nearby. The first sync downloads everything the ring holds, which takes a few minutes.</source>
        <translation>Ei vielä historiaa. Vedä alas ja valitse Synkronoi nyt sormus lähellä. Ensimmäinen synkronointi lataa kaiken, mitä sormus sisältää, ja kestää muutaman minuutin.</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="159"/>
        <source>Last night, %1</source>
        <translation>Viime yö, %1</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="173"/>
        <source>Sleep</source>
        <translation>Uni</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="195"/>
        <source>asleep (estimated)</source>
        <translation>unessa (arvio)</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="195"/>
        <source>asleep</source>
        <translation>unessa</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="203"/>
        <source>Efficiency %1 %</source>
        <translation>Tehokkuus %1 %</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="224"/>
        <source>Lowest heart rate</source>
        <translation>Alin syke</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="230"/>
        <source>HRV</source>
        <translation>HRV</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="239"/>
        <source>Skin temp. vs usual</source>
        <translation>Ihon lämpö vs. tavallinen</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="239"/>
        <source>Skin temperature</source>
        <translation>Ihon lämpötila</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="245"/>
        <source>Blood oxygen</source>
        <translation>Veren happi</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="251"/>
        <source>Breathing</source>
        <translation>Hengitys</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="258"/>
        <source>Heart rate, %1 ago</source>
        <translation>Syke, %1 sitten</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="259"/>
        <source>Heart rate</source>
        <translation>Syke</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="270"/>
        <source>Today</source>
        <translation>Tänään</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="285"/>
        <source>Heart rate by hour</source>
        <translation>Syke tunneittain</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="298"/>
        <source>About %1 kcal active · %2 min moderate or harder</source>
        <translation>Noin %1 kcal aktiivista · %2 min kohtalaista tai rankempaa</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="313"/>
        <source>Sleep score, recent nights</source>
        <translation>Unipisteet, viime yöt</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="347"/>
        <source>Sleep history</source>
        <translation>Unihistoria</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="357"/>
        <source>Activity history</source>
        <translation>Aktiivisuushistoria</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="367"/>
        <source>Trends</source>
        <translation>Trendit</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="381"/>
        <source>%1 · battery %2 % · firmware %3</source>
        <translation>%1 · akku %2 % · laiteohjelmisto %3</translation>
    </message>
</context>
<context>
    <name>NightPage</name>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="21"/>
        <source>Duration</source>
        <translation>Kesto</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="22"/>
        <location filename="../qml/pages/NightPage.qml" line="124"/>
        <source>Efficiency</source>
        <translation>Tehokkuus</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="23"/>
        <source>Falling asleep</source>
        <translation>Nukahtaminen</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="24"/>
        <source>Time awake in the night</source>
        <translation>Hereillä yöllä</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="25"/>
        <location filename="../qml/pages/NightPage.qml" line="144"/>
        <source>Awakenings</source>
        <translation>Heräämiset</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="26"/>
        <source>Deep and REM sleep</source>
        <translation>Syvä ja REM-uni</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="27"/>
        <source>Heart rate and HRV vs usual</source>
        <translation>Syke ja HRV vs. tavallinen</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="32"/>
        <source>Optimal</source>
        <translation>Erinomainen</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="33"/>
        <source>Good</source>
        <translation>Hyvä</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="34"/>
        <source>Fair</source>
        <translation>Kohtalainen</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="35"/>
        <source>Pay attention</source>
        <translation>Kiinnitä huomiota</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="59"/>
        <source>No data for this night</source>
        <translation>Ei tietoja tältä yöltä</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="74"/>
        <source>Sleep</source>
        <translation>Uni</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="96"/>
        <source>asleep, estimated</source>
        <translation>unessa, arvio</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="96"/>
        <source>asleep</source>
        <translation>unessa</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="102"/>
        <source>%1 in bed</source>
        <translation>%1 sängyssä</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="131"/>
        <source>To fall asleep</source>
        <translation>Nukahtamiseen</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="138"/>
        <source>Awake in the night</source>
        <translation>Hereillä yöllä</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="151"/>
        <source>Lowest heart rate</source>
        <translation>Alin syke</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="158"/>
        <source>HRV</source>
        <translation>HRV</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="165"/>
        <source>Sleep stages</source>
        <translation>Unen vaiheet</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="180"/>
        <source>Deep</source>
        <translation>Syvä</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="181"/>
        <source>Light</source>
        <translation>Kevyt</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="182"/>
        <source>REM</source>
        <translation>REM</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="183"/>
        <source>Awake</source>
        <translation>Hereillä</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="192"/>
        <source>The ring has not reported sleep stages for this night. Its own sleep window is shown, and the time asleep is estimated from it.</source>
        <translation>Sormus ei ole raportoinut unen vaiheita tältä yöltä. Näytetään sormuksen oma unijakso, ja nukuttu aika arvioidaan siitä.</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="199"/>
        <source>Heart rate (bpm)</source>
        <translation>Syke (bpm)</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="212"/>
        <source>Heart rate variability (ms)</source>
        <translation>Sykevaihtelu (ms)</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="225"/>
        <source>Breathing (per minute)</source>
        <translation>Hengitys (minuutissa)</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="240"/>
        <source>Skin temperature (°C), %1 vs usual</source>
        <translation>Ihon lämpötila (°C), %1 vs. tavallinen</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="241"/>
        <source>Skin temperature (°C)</source>
        <translation>Ihon lämpötila (°C)</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="255"/>
        <source>Blood oxygen (%)</source>
        <translation>Veren happi (%)</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="271"/>
        <source>How the score is made</source>
        <translation>Miten pisteet muodostuvat</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="321"/>
        <source>This score is this app&apos;s own estimate, built from published sleep research (National Sleep Foundation guidelines). Oura computes its score with closed software on the phone, so the two numbers will not match.</source>
        <translation>Nämä pisteet ovat tämän sovelluksen oma arvio, joka perustuu julkaistuun unitutkimukseen (National Sleep Foundationin suositukset). Oura laskee omat pisteensä puhelimessa suljetulla ohjelmistolla, joten luvut eivät täsmää.</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="332"/>
        <source>The ring&apos;s clock could not be tied to the calendar for this night, so its times are placed from the moment of the download and may be off by hours.</source>
        <translation>Sormuksen kelloa ei saatu sidottua kalenteriin tältä yöltä, joten ajat on sijoitettu latauksen hetken mukaan ja ne voivat olla tuntien päässä.</translation>
    </message>
</context>
<context>
    <name>NightsPage</name>
    <message>
        <location filename="../qml/pages/NightsPage.qml" line="24"/>
        <source>Sleep</source>
        <translation>Uni</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightsPage.qml" line="25"/>
        <source>%1 nights</source>
        <translation>%1 yötä</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightsPage.qml" line="43"/>
        <source>nap</source>
        <translation>päiväunet</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightsPage.qml" line="53"/>
        <source>%1 % efficiency</source>
        <translation>%1 % tehokkuus</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightsPage.qml" line="72"/>
        <source>No nights yet</source>
        <translation>Ei vielä öitä</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightsPage.qml" line="73"/>
        <source>Sync the ring after a night&apos;s sleep</source>
        <translation>Synkronoi sormus yöunen jälkeen</translation>
    </message>
</context>
<context>
    <name>RingLink</name>
    <message>
        <location filename="../src/ble/ringlink.cpp" line="123"/>
        <source>Pairing timed out. Is the ring close to the phone and not connected to another device?</source>
        <translation>Pariutus aikakatkaistiin. Onko sormus lähellä puhelinta eikä yhdistettynä toiseen laitteeseen?</translation>
    </message>
    <message>
        <location filename="../src/ble/ringlink.cpp" line="129"/>
        <source>The ring refused to pair (%1). A ring that is still set up with the Oura app must be factory reset first.</source>
        <translation>Sormus kieltäytyi pariutumasta (%1). Sormus, joka on yhä otettu käyttöön Oura-sovelluksella, täytyy ensin palauttaa tehdasasetuksiin.</translation>
    </message>
    <message>
        <location filename="../src/ble/ringlink.cpp" line="131"/>
        <source>Pairing failed: %1

If this keeps happening, pair the ring once in Settings &gt; Bluetooth, then set it up here.</source>
        <translation>Pariutus epäonnistui: %1

Jos tämä toistuu, pariuta sormus kerran kohdassa Asetukset &gt; Bluetooth ja ota se sitten käyttöön täällä.</translation>
    </message>
    <message>
        <location filename="../src/ble/ringlink.cpp" line="171"/>
        <source>Could not reach the ring. Is it nearby, charged, and not connected to another phone?</source>
        <translation>Sormukseen ei saatu yhteyttä. Onko se lähellä, ladattu eikä yhdistettynä toiseen puhelimeen?</translation>
    </message>
    <message>
        <location filename="../src/ble/ringlink.cpp" line="175"/>
        <source>Could not connect to the ring: %1</source>
        <translation>Sormukseen yhdistäminen epäonnistui: %1</translation>
    </message>
    <message>
        <location filename="../src/ble/ringlink.cpp" line="181"/>
        <source>The ring connected but did not finish Bluetooth service discovery.</source>
        <translation>Sormus yhdistyi mutta ei suorittanut Bluetooth-palveluiden etsintää loppuun.</translation>
    </message>
    <message>
        <location filename="../src/ble/ringlink.cpp" line="201"/>
        <source>The ring refused notifications because the link is not encrypted. Pair the ring first.</source>
        <translation>Sormus kieltäytyi ilmoituksista, koska yhteys ei ole salattu. Pariuta sormus ensin.</translation>
    </message>
    <message>
        <location filename="../src/ble/ringlink.cpp" line="202"/>
        <source>Could not subscribe to ring notifications: %1</source>
        <translation>Sormuksen ilmoituksia ei voitu tilata: %1</translation>
    </message>
    <message>
        <location filename="../src/ble/ringlink.cpp" line="214"/>
        <source>Could not list Bluetooth services: %1</source>
        <translation>Bluetooth-palveluita ei voitu listata: %1</translation>
    </message>
    <message>
        <location filename="../src/ble/ringlink.cpp" line="231"/>
        <source>This device does not offer the Oura ring service.</source>
        <translation>Tämä laite ei tarjoa Oura-sormuksen palvelua.</translation>
    </message>
    <message>
        <location filename="../src/ble/ringlink.cpp" line="247"/>
        <source>The ring service is missing its read or write characteristic.</source>
        <translation>Sormuspalvelusta puuttuu luku- tai kirjoitusominaisuus.</translation>
    </message>
    <message>
        <location filename="../src/ble/ringlink.cpp" line="330"/>
        <source>The ring is not connected</source>
        <translation>Sormus ei ole yhdistettynä</translation>
    </message>
    <message>
        <location filename="../src/ble/ringlink.cpp" line="342"/>
        <source>Could not write to the ring: %1</source>
        <translation>Sormukseen ei voitu kirjoittaa: %1</translation>
    </message>
    <message>
        <location filename="../src/ble/ringlink.cpp" line="387"/>
        <source>Bluetooth link to the ring was lost</source>
        <translation>Bluetooth-yhteys sormukseen katkesi</translation>
    </message>
</context>
<context>
    <name>RingScanner</name>
    <message>
        <location filename="../src/ble/ringscanner.cpp" line="54"/>
        <source>Could not reach Bluetooth (org.bluez): %1</source>
        <translation>Bluetoothiin ei saatu yhteyttä (org.bluez): %1</translation>
    </message>
    <message>
        <location filename="../src/ble/ringscanner.cpp" line="65"/>
        <location filename="../src/ble/ringscanner.cpp" line="162"/>
        <source>No Bluetooth adapter found</source>
        <translation>Bluetooth-sovitinta ei löytynyt</translation>
    </message>
    <message>
        <location filename="../src/ble/ringscanner.cpp" line="94"/>
        <source>Could not list Bluetooth devices: %1</source>
        <translation>Bluetooth-laitteita ei voitu listata: %1</translation>
    </message>
    <message>
        <location filename="../src/ble/ringscanner.cpp" line="122"/>
        <source>Could not start scanning: %1</source>
        <translation>Skannausta ei voitu aloittaa: %1</translation>
    </message>
</context>
<context>
    <name>SettingsPage</name>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="16"/>
        <source>Settings</source>
        <translation>Asetukset</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="18"/>
        <source>About you</source>
        <translation>Sinusta</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="26"/>
        <source>Used only for the sleep score&apos;s age bands and for the calorie estimate. It is not sent to the ring and does not leave the phone.</source>
        <translation>Käytetään vain unipisteiden ikäryhmiin ja kalorien arvioon. Sitä ei lähetetä sormukseen eikä se poistu puhelimesta.</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="33"/>
        <source>Age, years</source>
        <translation>Ikä, vuotta</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="43"/>
        <source>Weight, kg</source>
        <translation>Paino, kg</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="53"/>
        <source>Ring</source>
        <translation>Sormus</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="57"/>
        <source>Name</source>
        <translation>Nimi</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="62"/>
        <source>Serial</source>
        <translation>Sarjanumero</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="67"/>
        <source>Firmware</source>
        <translation>Laiteohjelmisto</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="72"/>
        <source>Battery</source>
        <translation>Akku</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="77"/>
        <source>Last sync</source>
        <translation>Viimeisin synkronointi</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="86"/>
        <source>Back up the key</source>
        <translation>Varmuuskopioi avain</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="93"/>
        <source>Diagnostics</source>
        <translation>Diagnostiikka</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="102"/>
        <source>Forget this ring</source>
        <translation>Unohda tämä sormus</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="103"/>
        <source>Forgetting the ring and its history</source>
        <translation>Unohdetaan sormus ja sen historia</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="119"/>
        <source>Forgetting removes the stored history and the ring&apos;s key from this phone. The ring keeps its key, so back it up first if you may want to read the ring again.</source>
        <translation>Unohtaminen poistaa tallennetun historian ja sormuksen avaimen tästä puhelimesta. Sormus säilyttää avaimensa, joten varmuuskopioi se ensin, jos saatat haluta lukea sormuksen uudelleen.</translation>
    </message>
</context>
<context>
    <name>SetupPage</name>
    <message>
        <location filename="../qml/pages/SetupPage.qml" line="56"/>
        <source>Set up ring</source>
        <translation>Ota sormus käyttöön</translation>
    </message>
    <message>
        <location filename="../qml/pages/SetupPage.qml" line="65"/>
        <source>A ring has one owner. To use it here it must be factory reset first: remove it from the Oura app, then reset it (on a Gen 3 or 4 dock: place the ring, wait two seconds, flip the dock upside down, up, down and up again, waiting for the light to turn blue, red, magenta and yellow). Do not finish the Oura app&apos;s set-up afterwards: that installs Oura&apos;s own key and locks this app out.</source>
        <translation>Sormuksella on yksi omistaja. Jotta voit käyttää sitä täällä, se täytyy ensin palauttaa tehdasasetuksiin: poista se Oura-sovelluksesta ja nollaa se (Gen 3 tai 4 -telakassa: aseta sormus telakkaan, odota kaksi sekuntia, käännä telakka ylösalaisin, takaisin, taas ylösalaisin ja takaisin, ja odota joka kerta valon muuttuvan siniseksi, punaiseksi, magentaksi ja keltaiseksi). Älä suorita Oura-sovelluksen käyttöönottoa loppuun sen jälkeen: se asentaa Ouran oman avaimen ja lukitsee tämän sovelluksen ulos.</translation>
    </message>
    <message>
        <location filename="../qml/pages/SetupPage.qml" line="78"/>
        <source>After set-up the Oura app can no longer use this ring until it is reset again.</source>
        <translation>Käyttöönoton jälkeen Oura-sovellus ei voi käyttää tätä sormusta, ennen kuin se nollataan uudelleen.</translation>
    </message>
    <message>
        <location filename="../qml/pages/SetupPage.qml" line="83"/>
        <source>My ring already has a key</source>
        <translation>Sormuksellani on jo avain</translation>
    </message>
    <message>
        <location filename="../qml/pages/SetupPage.qml" line="84"/>
        <source>Use this after reinstalling the app, with the key you saved</source>
        <translation>Käytä tätä sovelluksen uudelleenasennuksen jälkeen tallentamallasi avaimella</translation>
    </message>
    <message>
        <location filename="../qml/pages/SetupPage.qml" line="94"/>
        <source>Key, 32 hexadecimal characters</source>
        <translation>Avain, 32 heksadesimaalimerkkiä</translation>
    </message>
    <message>
        <location filename="../qml/pages/SetupPage.qml" line="103"/>
        <source>Looking for rings…</source>
        <translation>Etsitään sormuksia…</translation>
    </message>
    <message>
        <location filename="../qml/pages/SetupPage.qml" line="103"/>
        <source>Rings nearby</source>
        <translation>Sormukset lähellä</translation>
    </message>
    <message>
        <location filename="../qml/pages/SetupPage.qml" line="113"/>
        <source>None yet. Keep the ring close to the phone and make sure no other phone is connected to it. A factory-reset ring starts advertising once the reset has finished.</source>
        <translation>Ei vielä yhtään. Pidä sormus lähellä puhelinta ja varmista, ettei mikään toinen puhelin ole yhdistettynä siihen. Tehdasasetuksiin palautettu sormus alkaa mainostaa itseään, kun nollaus on valmis.</translation>
    </message>
    <message>
        <location filename="../qml/pages/SetupPage.qml" line="159"/>
        <source>Setting up %1</source>
        <translation>Otetaan käyttöön: %1</translation>
    </message>
    <message>
        <location filename="../qml/pages/SetupPage.qml" line="179"/>
        <source>paired</source>
        <translation>pariutettu</translation>
    </message>
    <message>
        <location filename="../qml/pages/SetupPage.qml" line="180"/>
        <source>this ring</source>
        <translation>tämä sormus</translation>
    </message>
</context>
<context>
    <name>TrendsPage</name>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="10"/>
        <source>Sleep score</source>
        <translation>Unipisteet</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="11"/>
        <source>Time asleep</source>
        <translation>Nukuttu aika</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="12"/>
        <source>Sleep efficiency</source>
        <translation>Unen tehokkuus</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="13"/>
        <source>Deep sleep</source>
        <translation>Syvä uni</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="14"/>
        <source>REM sleep</source>
        <translation>REM-uni</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="15"/>
        <source>Lowest heart rate</source>
        <translation>Alin syke</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="16"/>
        <source>HRV</source>
        <translation>HRV</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="17"/>
        <source>Skin temperature vs usual</source>
        <translation>Ihon lämpötila vs. tavallinen</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="18"/>
        <source>Skin temperature</source>
        <translation>Ihon lämpötila</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="19"/>
        <source>Blood oxygen</source>
        <translation>Veren happi</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="20"/>
        <source>Breathing rate</source>
        <translation>Hengitystiheys</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="59"/>
        <source>Trends</source>
        <translation>Trendit</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="62"/>
        <source>Measure</source>
        <translation>Mittaus</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="76"/>
        <source>Period</source>
        <translation>Jakso</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="79"/>
        <source>14 nights</source>
        <translation>14 yötä</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="80"/>
        <source>30 nights</source>
        <translation>30 yötä</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="81"/>
        <source>90 nights</source>
        <translation>90 yötä</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="111"/>
        <source>Lowest</source>
        <translation>Alin</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="117"/>
        <source>Average</source>
        <translation>Keskiarvo</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="123"/>
        <source>Highest</source>
        <translation>Korkein</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="133"/>
        <source>Nothing to show for this measure yet.</source>
        <translation>Tästä mittauksesta ei ole vielä mitään näytettävää.</translation>
    </message>
</context>
</TS>
