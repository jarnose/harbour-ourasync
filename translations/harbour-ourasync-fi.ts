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
        <source>Reading the ring needs no network. The history is kept in a database on this phone, and the ring&apos;s key and any Oura sign-in in the Sailfish secrets vault. The only time the app uses the network is when you tap Import in the Oura cloud page, and then it only fetches from Oura: nothing is uploaded anywhere.</source>
        <translation>Sormuksen lukeminen ei tarvitse verkkoa. Historia on tietokannassa tässä puhelimessa, ja sormuksen avain sekä mahdollinen Oura-kirjautuminen Sailfishin salaisuusholvissa. Verkkoa käytetään vain, kun painat Tuo Oura-pilvisivulla, ja silloinkin vain haetaan Ouralta: mitään ei lähetetä minnekään.</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="61"/>
        <source>Version</source>
        <translation>Versio</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="65"/>
        <source>Credits</source>
        <translation>Tekijät ja kiitokset</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="73"/>
        <source>The ring&apos;s protocol was reverse engineered by the open_oura project, and the sleep-score method and several decoders come from open_health, both by Thomas Marchand (MIT licence). The dashboards follow the ideas of Cracked Oura. The protocol details were checked against version 7.27.0 of Oura&apos;s Android app.</source>
        <translation>Sormuksen protokollan on selvittänyt open_oura-projekti, ja unipisteiden menetelmä sekä useat dekooderit tulevat open_health-projektista; molemmat ovat Thomas Marchandin (MIT-lisenssi). Näkymät seuraavat Cracked Ouran ideoita. Protokollan yksityiskohdat tarkistettiin Ouran Android-sovelluksen version 7.27.0 koodista.</translation>
    </message>
</context>
<context>
    <name>AppController</name>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="300"/>
        <location filename="../src/controller/appcontroller.cpp" line="1534"/>
        <source>Imported</source>
        <translation>Tuotu</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="332"/>
        <source>Could not open the local database: %1</source>
        <translation>Paikallista tietokantaa ei voitu avata: %1</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="391"/>
        <source>Another operation is still running.</source>
        <translation>Toinen toiminto on vielä käynnissä.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="408"/>
        <source>Done</source>
        <translation>Valmis</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="547"/>
        <location filename="../src/controller/appcontroller.cpp" line="579"/>
        <source>The secrets vault did not answer.</source>
        <translation>Salaisuusholvi ei vastannut.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="599"/>
        <source>The stored key has the wrong length.</source>
        <translation>Tallennetun avaimen pituus on väärä.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="630"/>
        <source>Connecting to the ring…</source>
        <translation>Yhdistetään sormukseen…</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="639"/>
        <source>The ring&apos;s key is not in the secrets vault (%1). Set the ring up again, or enter its key.</source>
        <translation>Sormuksen avainta ei löydy salaisuusholvista (%1). Ota sormus käyttöön uudelleen tai anna sen avain.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="640"/>
        <source>not found</source>
        <translation>ei löytynyt</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="642"/>
        <location filename="../src/controller/appcontroller.cpp" line="813"/>
        <location filename="../src/controller/appcontroller.cpp" line="854"/>
        <source>Authenticating…</source>
        <translation>Todennetaan…</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="648"/>
        <source>The ring has been factory reset and has no key. Set it up again.</source>
        <translation>Sormus on palautettu tehdasasetuksiin eikä sillä ole avainta. Ota se käyttöön uudelleen.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="650"/>
        <source>The ring rejected the stored key. Another app has probably set the ring up since; a factory reset and a new set-up are needed.</source>
        <translation>Sormus hylkäsi tallennetun avaimen. Jokin toinen sovellus on luultavasti ottanut sormuksen käyttöön sen jälkeen; tarvitaan tehdasasetusten palautus ja uusi käyttöönotto.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="652"/>
        <source>Authentication failed: %1</source>
        <translation>Todennus epäonnistui: %1</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="672"/>
        <source>Preparing the ring…</source>
        <translation>Valmistellaan sormusta…</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="675"/>
        <source>Could not prepare the ring: %1</source>
        <translation>Sormuksen valmistelu epäonnistui: %1</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="681"/>
        <source>Could not set the ring&apos;s clock: %1</source>
        <translation>Sormuksen kelloa ei voitu asettaa: %1</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="705"/>
        <source>Downloading history…</source>
        <translation>Ladataan historiaa…</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="713"/>
        <source>Downloading history: %1 events</source>
        <translation>Ladataan historiaa: %1 tapahtumaa</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="719"/>
        <source>History download stopped: %1</source>
        <translation>Historian lataus pysähtyi: %1</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="760"/>
        <location filename="../src/controller/appcontroller.cpp" line="903"/>
        <source>No ring is set up yet.</source>
        <translation>Sormusta ei ole vielä otettu käyttöön.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="763"/>
        <source>Syncing…</source>
        <translation>Synkronoidaan…</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="770"/>
        <source>Bluetooth does not know this ring any more. Set it up again.</source>
        <translation>Bluetooth ei enää tunne tätä sormusta. Ota se käyttöön uudelleen.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="788"/>
        <location filename="../src/controller/appcontroller.cpp" line="885"/>
        <source>Pairing…</source>
        <translation>Pariutetaan…</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="796"/>
        <location filename="../src/controller/appcontroller.cpp" line="876"/>
        <source>Oura ring</source>
        <translation>Oura-sormus</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="818"/>
        <source>Installing the ring&apos;s key…</source>
        <translation>Asennetaan sormuksen avainta…</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="822"/>
        <source>Could not read the system&apos;s random source.</source>
        <translation>Järjestelmän satunnaislähdettä ei voitu lukea.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="831"/>
        <location filename="../src/controller/appcontroller.cpp" line="880"/>
        <source>Could not store the key in the secrets vault: %1</source>
        <translation>Avainta ei voitu tallentaa salaisuusholviin: %1</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="841"/>
        <source>The ring would not take a new key: %1

A ring that is still set up with the Oura app must first be factory reset.</source>
        <translation>Sormus ei ottanut uutta avainta: %1

Sormus, joka on yhä otettu käyttöön Oura-sovelluksella, täytyy ensin palauttaa tehdasasetuksiin.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="870"/>
        <source>A key is 32 hexadecimal characters (16 bytes).</source>
        <translation>Avain on 32 heksadesimaalimerkkiä (16 tavua).</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="873"/>
        <source>Storing the key…</source>
        <translation>Tallennetaan avainta…</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="906"/>
        <source>Syncing before the reset…</source>
        <translation>Synkronoidaan ennen nollausta…</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="913"/>
        <source>Bluetooth does not know this ring any more. Reset it with the charger instead.</source>
        <translation>Bluetooth ei enää tunne tätä sormusta. Nollaa se latauslaitteella.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="924"/>
        <source>Resetting the ring…</source>
        <translation>Nollataan sormusta…</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="1014"/>
        <source>Could not create the folder %1.</source>
        <translation>Kansiota %1 ei voitu luoda.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="1026"/>
        <source>Could not copy the database to %1.</source>
        <translation>Tietokantaa ei voitu kopioida kohteeseen %1.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="1033"/>
        <source>There is nothing to export yet.</source>
        <translation>Vietävää ei ole vielä.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="1043"/>
        <source>Could not write %1: %2</source>
        <translation>Tiedostoa %1 ei voitu kirjoittaa: %2</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="1052"/>
        <source>Could not write the files: %1</source>
        <translation>Tiedostoja ei voitu kirjoittaa: %1</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="1082"/>
        <source>That is not a backup of this app&apos;s database.</source>
        <translation>Tämä ei ole tämän sovelluksen tietokannan varmuuskopio.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="1089"/>
        <source>Could not stage the backup.</source>
        <translation>Varmuuskopiota ei voitu valmistella.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="1403"/>
        <source>Could not store the Oura sign-in in the secrets vault: %1</source>
        <translation>Oura-kirjautumista ei voitu tallentaa salaisuusholviin: %1</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="1417"/>
        <location filename="../src/controller/appcontroller.cpp" line="1488"/>
        <source>Another import is still running.</source>
        <translation>Toinen tuonti on vielä käynnissä.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="1422"/>
        <source>Reading the export…</source>
        <translation>Luetaan vientiä…</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="1473"/>
        <source>Signing in to Oura…</source>
        <translation>Kirjaudutaan Ouraan…</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="1482"/>
        <source>Connected to Oura</source>
        <translation>Yhdistetty Ouraan</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="1492"/>
        <source>Not connected to Oura.</source>
        <translation>Ei yhteyttä Ouraan.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="1497"/>
        <source>Connecting to Oura…</source>
        <translation>Yhdistetään Ouraan…</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="1522"/>
        <source>Saving…</source>
        <translation>Tallennetaan…</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="1525"/>
        <source>Could not save the import: %1</source>
        <translation>Tuontia ei voitu tallentaa: %1</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="1527"/>
        <source>The import stopped early, and what was fetched so far was saved. %1</source>
        <translation>Tuonti keskeytyi aikaisin, ja tähän mennessä haettu tallennettiin. %1</translation>
    </message>
</context>
<context>
    <name>CloudPage</name>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="43"/>
        <source>Oura cloud</source>
        <translation>Oura-pilvi</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="53"/>
        <source>%1 nights, %2 days of activity and %3 readiness scores, from %4 to %5.</source>
        <translation>%1 yötä, %2 päivää aktiivisuutta ja %3 valmiuspistettä, ajalta %4–%5.</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="56"/>
        <source>Nothing imported yet.</source>
        <translation>Ei vielä tuotua dataa.</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="96"/>
        <source>Data export (no account needed)</source>
        <translation>Datavienti (ei tiliä tarvita)</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="104"/>
        <source>Every Oura user can download their own data from membership.ouraring.com/data-export (sign in with the email code, choose Request export, wait for the email). Save the ZIP on the phone and open it here: the scores, sleep stages, heart rate and activity of all your history are read, and nothing leaves the phone.</source>
        <translation>Jokainen Oura-käyttäjä voi ladata oman datansa osoitteesta membership.ouraring.com/data-export (kirjaudu sähköpostikoodilla, valitse Request export ja odota sähköpostia). Tallenna ZIP puhelimeen ja avaa se tässä: koko historiasi pisteet, unen vaiheet, syke ja aktiivisuus luetaan, eikä mikään poistu puhelimesta.</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="113"/>
        <source>Open from Downloads</source>
        <translation>Avaa Latauksista</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="120"/>
        <source>Open from Documents</source>
        <translation>Avaa Asiakirjoista</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="127"/>
        <source>Or a file or folder path</source>
        <translation>Tai tiedoston tai kansion polku</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="139"/>
        <source>Oura Cloud API</source>
        <translation>Oura Cloud API</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="147"/>
        <source>Needs an active Oura Membership. Oura no longer issues personal tokens, so you register a small application of your own at developer.ouraring.com/applications with the redirect address below, and enter its client id and secret here. They stay in this phone&apos;s secure vault.</source>
        <translation>Vaatii voimassa olevan Oura-jäsenyyden. Oura ei enää anna henkilökohtaisia tunnisteita, joten rekisteröit osoitteessa developer.ouraring.com/applications oman pienen sovelluksen alla näkyvällä uudelleenohjausosoitteella ja annat sen client id:n ja salaisuuden tässä. Ne pysyvät tämän puhelimen suojatussa holvissa.</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="156"/>
        <source>Client id</source>
        <translation>Client id</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="166"/>
        <source>Client secret</source>
        <translation>Client secret</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="174"/>
        <source>Redirect address: http://localhost:%1/callback</source>
        <translation>Uudelleenohjausosoite: http://localhost:%1/callback</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="185"/>
        <source>Sign in again</source>
        <translation>Kirjaudu uudelleen</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="185"/>
        <source>Sign in with Oura</source>
        <translation>Kirjaudu Ouralla</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="197"/>
        <source>If the browser could not reach the app: paste the address or code</source>
        <translation>Jos selain ei tavoittanut sovellusta: liitä osoite tai koodi</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="198"/>
        <source>Address or code</source>
        <translation>Osoite tai koodi</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="214"/>
        <source>Connected to Oura.</source>
        <translation>Yhdistetty Ouraan.</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="220"/>
        <source>Import</source>
        <translation>Tuo</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="222"/>
        <source>the last 30 days</source>
        <translation>viimeiset 30 päivää</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="223"/>
        <source>the last 90 days</source>
        <translation>viimeiset 90 päivää</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="224"/>
        <source>the last year</source>
        <translation>viimeisin vuosi</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="225"/>
        <source>everything (up to 10 years)</source>
        <translation>kaikki (enintään 10 vuotta)</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="232"/>
        <source>Import now</source>
        <translation>Tuo nyt</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="243"/>
        <source>Disconnect</source>
        <translation>Katkaise yhteys</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="250"/>
        <source>Imported data</source>
        <translation>Tuotu data</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="255"/>
        <source>Check my scores against Oura&apos;s</source>
        <translation>Vertaa pisteitäni Ouran pisteisiin</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="263"/>
        <source>Remove imported data</source>
        <translation>Poista tuotu data</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="264"/>
        <source>Removing imported data</source>
        <translation>Poistetaan tuotua dataa</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="274"/>
        <source>Removing leaves the history read from the ring untouched.</source>
        <translation>Poisto ei koske sormuksesta luettuun historiaan.</translation>
    </message>
    <message>
        <location filename="../qml/pages/CloudPage.qml" line="285"/>
        <location filename="../qml/pages/CloudPage.qml" line="295"/>
        <source>Choose the export</source>
        <translation>Valitse vienti</translation>
    </message>
</context>
<context>
    <name>ContributorBars</name>
    <message>
        <location filename="../qml/components/ContributorBars.qml" line="16"/>
        <source>Total sleep</source>
        <translation>Kokonaisuni</translation>
    </message>
    <message>
        <location filename="../qml/components/ContributorBars.qml" line="17"/>
        <source>Deep sleep</source>
        <translation>Syvä uni</translation>
    </message>
    <message>
        <location filename="../qml/components/ContributorBars.qml" line="18"/>
        <source>REM sleep</source>
        <translation>REM-uni</translation>
    </message>
    <message>
        <location filename="../qml/components/ContributorBars.qml" line="19"/>
        <source>Efficiency</source>
        <translation>Tehokkuus</translation>
    </message>
    <message>
        <location filename="../qml/components/ContributorBars.qml" line="20"/>
        <source>Latency</source>
        <translation>Nukahtamisaika</translation>
    </message>
    <message>
        <location filename="../qml/components/ContributorBars.qml" line="21"/>
        <source>Restfulness</source>
        <translation>Levollisuus</translation>
    </message>
    <message>
        <location filename="../qml/components/ContributorBars.qml" line="22"/>
        <source>Restfulness part</source>
        <translation>Levollisuuden osa</translation>
    </message>
    <message>
        <location filename="../qml/components/ContributorBars.qml" line="23"/>
        <source>Timing</source>
        <translation>Ajoitus</translation>
    </message>
    <message>
        <location filename="../qml/components/ContributorBars.qml" line="25"/>
        <source>Activity balance</source>
        <translation>Aktiivisuuden tasapaino</translation>
    </message>
    <message>
        <location filename="../qml/components/ContributorBars.qml" line="26"/>
        <source>Body temperature</source>
        <translation>Kehon lämpötila</translation>
    </message>
    <message>
        <location filename="../qml/components/ContributorBars.qml" line="27"/>
        <source>HRV balance</source>
        <translation>HRV-tasapaino</translation>
    </message>
    <message>
        <location filename="../qml/components/ContributorBars.qml" line="28"/>
        <source>Previous day activity</source>
        <translation>Edellisen päivän aktiivisuus</translation>
    </message>
    <message>
        <location filename="../qml/components/ContributorBars.qml" line="29"/>
        <source>Previous night</source>
        <translation>Edellinen yö</translation>
    </message>
    <message>
        <location filename="../qml/components/ContributorBars.qml" line="30"/>
        <source>Recovery index</source>
        <translation>Palautumisindeksi</translation>
    </message>
    <message>
        <location filename="../qml/components/ContributorBars.qml" line="31"/>
        <source>Resting heart rate</source>
        <translation>Leposyke</translation>
    </message>
    <message>
        <location filename="../qml/components/ContributorBars.qml" line="32"/>
        <source>Sleep balance</source>
        <translation>Unen tasapaino</translation>
    </message>
    <message>
        <location filename="../qml/components/ContributorBars.qml" line="34"/>
        <source>Daily targets</source>
        <translation>Päivätavoitteet</translation>
    </message>
    <message>
        <location filename="../qml/components/ContributorBars.qml" line="35"/>
        <source>Move every hour</source>
        <translation>Liiku joka tunti</translation>
    </message>
    <message>
        <location filename="../qml/components/ContributorBars.qml" line="36"/>
        <source>Recovery time</source>
        <translation>Palautumisaika</translation>
    </message>
    <message>
        <location filename="../qml/components/ContributorBars.qml" line="37"/>
        <source>Stay active</source>
        <translation>Pysy aktiivisena</translation>
    </message>
    <message>
        <location filename="../qml/components/ContributorBars.qml" line="38"/>
        <source>Training frequency</source>
        <translation>Harjoittelun tiheys</translation>
    </message>
    <message>
        <location filename="../qml/components/ContributorBars.qml" line="39"/>
        <source>Training volume</source>
        <translation>Harjoittelun määrä</translation>
    </message>
</context>
<context>
    <name>CoverPage</name>
    <message>
        <location filename="../qml/cover/CoverPage.qml" line="58"/>
        <source>Slept %1</source>
        <translation>Nukuttu %1</translation>
    </message>
    <message>
        <location filename="../qml/cover/CoverPage.qml" line="75"/>
        <source>No history yet</source>
        <translation>Ei vielä historiaa</translation>
    </message>
    <message>
        <location filename="../qml/cover/CoverPage.qml" line="75"/>
        <source>No ring set up</source>
        <translation>Sormusta ei otettu käyttöön</translation>
    </message>
</context>
<context>
    <name>DayPage</name>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="40"/>
        <source>Oura activity</source>
        <translation>Ouran aktiivisuus</translation>
    </message>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="46"/>
        <source>Oura readiness</source>
        <translation>Ouran valmius</translation>
    </message>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="52"/>
        <source>Steps</source>
        <translation>Askeleet</translation>
    </message>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="59"/>
        <source>Total burned</source>
        <translation>Kulutus yhteensä</translation>
    </message>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="66"/>
        <source>Temperature vs usual</source>
        <translation>Lämpötila vs. tavallinen</translation>
    </message>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="78"/>
        <source>Active, estimated</source>
        <translation>Aktiivinen, arvio</translation>
    </message>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="84"/>
        <source>Moderate or harder</source>
        <translation>Kohtalaista tai rankempaa</translation>
    </message>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="90"/>
        <source>Light</source>
        <translation>Kevyt</translation>
    </message>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="96"/>
        <source>Lowest heart rate</source>
        <translation>Alin syke</translation>
    </message>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="102"/>
        <source>Highest heart rate</source>
        <translation>Korkein syke</translation>
    </message>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="108"/>
        <source>Average effort</source>
        <translation>Keskimääräinen rasitus</translation>
    </message>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="119"/>
        <source>Activity (estimate)</source>
        <translation>Aktiivisuus (arvio)</translation>
    </message>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="123"/>
        <source>Activity, estimated contributors</source>
        <translation>Aktiivisuus, arvioidut osatekijät</translation>
    </message>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="136"/>
        <source>Worked out from the ring&apos;s movement over this and the six days before, against your goal in Settings. Recovery time is taken as full. It is close to Oura&apos;s score but not the same.</source>
        <translation>Laskettu sormuksen liikkeestä tämän ja kuuden edellisen päivän ajalta asetuksiin laittamaasi tavoitetta vasten. Palautumisaika oletetaan täydeksi. Se on lähellä Ouran pisteitä mutta ei sama.</translation>
    </message>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="142"/>
        <source>Readiness, Oura&apos;s contributors</source>
        <translation>Valmius, Ouran osatekijät</translation>
    </message>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="150"/>
        <source>Activity, Oura&apos;s contributors</source>
        <translation>Aktiivisuus, Ouran osatekijät</translation>
    </message>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="157"/>
        <source>Heart rate by hour (bpm)</source>
        <translation>Syke tunneittain (bpm)</translation>
    </message>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="165"/>
        <source>Effort by hour (MET)</source>
        <translation>Rasitus tunneittain (MET)</translation>
    </message>
    <message>
        <location filename="../qml/pages/DayPage.qml" line="180"/>
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
        <location filename="../qml/pages/DaysPage.qml" line="56"/>
        <source>%1 steps</source>
        <translation>%1 askelta</translation>
    </message>
    <message>
        <location filename="../qml/pages/DaysPage.qml" line="70"/>
        <source>%1 kcal</source>
        <translation>%1 kcal</translation>
    </message>
    <message>
        <location filename="../qml/pages/DaysPage.qml" line="76"/>
        <source>No activity yet</source>
        <translation>Ei vielä aktiivisuutta</translation>
    </message>
    <message>
        <location filename="../qml/pages/DaysPage.qml" line="77"/>
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
    <name>ExportPage</name>
    <message>
        <location filename="../qml/pages/ExportPage.qml" line="31"/>
        <source>Export and backup</source>
        <translation>Vienti ja varmuuskopio</translation>
    </message>
    <message>
        <location filename="../qml/pages/ExportPage.qml" line="39"/>
        <source>The files are written to %1 and go nowhere else.</source>
        <translation>Tiedostot kirjoitetaan kansioon %1, eivätkä ne mene minnekään muualle.</translation>
    </message>
    <message>
        <location filename="../qml/pages/ExportPage.qml" line="42"/>
        <source>For other programs</source>
        <translation>Muille ohjelmille</translation>
    </message>
    <message>
        <location filename="../qml/pages/ExportPage.qml" line="49"/>
        <source>FHIR is the standard other health systems read: one file with every night and day. CSV opens in a spreadsheet. Oura&apos;s own scores and this app&apos;s estimates are kept apart in both.</source>
        <translation>FHIR on standardi, jonka muut terveysjärjestelmät lukevat: yksi tiedosto, jossa on jokainen yö ja päivä. CSV aukeaa taulukkolaskentaan. Ouran omat pisteet ja tämän sovelluksen arviot pidetään kummassakin erillään.</translation>
    </message>
    <message>
        <location filename="../qml/pages/ExportPage.qml" line="55"/>
        <source>Export FHIR</source>
        <translation>Vie FHIR</translation>
    </message>
    <message>
        <location filename="../qml/pages/ExportPage.qml" line="60"/>
        <source>Export CSV</source>
        <translation>Vie CSV</translation>
    </message>
    <message>
        <location filename="../qml/pages/ExportPage.qml" line="66"/>
        <source>Backup</source>
        <translation>Varmuuskopio</translation>
    </message>
    <message>
        <location filename="../qml/pages/ExportPage.qml" line="73"/>
        <source>A backup is the whole database: the ring&apos;s history, what was imported from Oura and your settings. The ring&apos;s key is not in it; it stays in the secrets vault, so back that up separately (Settings, Back up the key).</source>
        <translation>Varmuuskopio on koko tietokanta: sormuksen historia, Ouralta tuotu tieto ja asetuksesi. Sormuksen avain ei ole mukana, vaan se pysyy salaisuusholvissa, joten varmuuskopioi se erikseen (Asetukset, Varmuuskopioi avain).</translation>
    </message>
    <message>
        <location filename="../qml/pages/ExportPage.qml" line="79"/>
        <source>Back up now</source>
        <translation>Varmuuskopioi nyt</translation>
    </message>
    <message>
        <location filename="../qml/pages/ExportPage.qml" line="106"/>
        <source>Restore</source>
        <translation>Palauta</translation>
    </message>
    <message>
        <location filename="../qml/pages/ExportPage.qml" line="115"/>
        <source>Tap a backup to restore it. It replaces what the app holds now, and takes effect when the app is started again.</source>
        <translation>Napauta varmuuskopiota palauttaaksesi sen. Se korvaa sen, mitä sovelluksessa nyt on, ja tulee voimaan, kun sovellus käynnistetään uudelleen.</translation>
    </message>
    <message>
        <location filename="../qml/pages/ExportPage.qml" line="124"/>
        <source>Restoring %1</source>
        <translation>Palautetaan %1</translation>
    </message>
    <message>
        <location filename="../qml/pages/ExportPage.qml" line="126"/>
        <source>Restore staged. Close the app and open it again.</source>
        <translation>Palautus on valmisteltu. Sulje sovellus ja avaa se uudelleen.</translation>
    </message>
</context>
<context>
    <name>Hypnogram</name>
    <message>
        <location filename="../qml/components/Hypnogram.qml" line="55"/>
        <source>Awake</source>
        <translation>Hereillä</translation>
    </message>
    <message>
        <location filename="../qml/components/Hypnogram.qml" line="55"/>
        <source>REM</source>
        <translation>REM</translation>
    </message>
    <message>
        <location filename="../qml/components/Hypnogram.qml" line="55"/>
        <source>Light</source>
        <translation>Kevyt</translation>
    </message>
    <message>
        <location filename="../qml/components/Hypnogram.qml" line="55"/>
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
        <location filename="../qml/components/LineChart.qml" line="83"/>
        <source>No data</source>
        <translation>Ei tietoja</translation>
    </message>
</context>
<context>
    <name>MainPage</name>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="50"/>
        <source>Not synced yet</source>
        <translation>Ei vielä synkronoitu</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="52"/>
        <source>Synced just now</source>
        <translation>Synkronoitu juuri nyt</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="52"/>
        <source>Synced %1 ago</source>
        <translation>Synkronoitu %1 sitten</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="62"/>
        <source>About</source>
        <translation>Tietoja</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="66"/>
        <source>Settings</source>
        <translation>Asetukset</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="75"/>
        <location filename="../qml/pages/MainPage.qml" line="159"/>
        <source>Set up ring</source>
        <translation>Ota sormus käyttöön</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="81"/>
        <source>Sync now</source>
        <translation>Synkronoi nyt</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="92"/>
        <source>Oura Sync</source>
        <translation>Oura Sync</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="145"/>
        <source>Your ring, without the cloud</source>
        <translation>Sormuksesi ilman pilveä</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="177"/>
        <source>Reading the history…</source>
        <translation>Luetaan historiaa…</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="178"/>
        <source>No history yet. Pull down and choose Sync now, with the ring nearby. The first sync downloads everything the ring holds, which takes a few minutes.</source>
        <translation>Ei vielä historiaa. Vedä alas ja valitse Synkronoi nyt sormus lähellä. Ensimmäinen synkronointi lataa kaiken, mitä sormus sisältää, ja kestää muutaman minuutin.</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="189"/>
        <source>Last night, %1</source>
        <translation>Viime yö, %1</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="317"/>
        <source>Sleep</source>
        <translation>Uni</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="226"/>
        <source>asleep (estimated)</source>
        <translation>unessa (arvio)</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="48"/>
        <source>From Oura&apos;s cloud</source>
        <translation>Ouran pilvestä</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="70"/>
        <source>Oura cloud</source>
        <translation>Oura-pilvi</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="152"/>
        <source>Oura Sync reads your ring over Bluetooth and shows its sleep, heart rate, HRV, temperature and activity. There is no account and no subscription: the data stays on this phone. You can also bring in your Oura history from a data export.</source>
        <translation>Oura Sync lukee sormuksesi Bluetoothilla ja näyttää sen unen, sykkeen, sykevaihtelun, lämpötilan ja aktiivisuuden. Ei tiliä eikä tilausta: tiedot pysyvät tässä puhelimessa. Voit tuoda myös Oura-historiasi datavientitiedostosta.</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="164"/>
        <source>Import Oura history</source>
        <translation>Tuo Oura-historia</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="204"/>
        <source>Oura</source>
        <translation>Oura</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="204"/>
        <source>Estimate</source>
        <translation>Arvio</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="226"/>
        <source>asleep</source>
        <translation>unessa</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="234"/>
        <source>Efficiency %1 %</source>
        <translation>Tehokkuus %1 %</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="249"/>
        <source>This app&apos;s estimate: %1</source>
        <translation>Tämän sovelluksen arvio: %1</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="263"/>
        <source>Lowest heart rate</source>
        <translation>Alin syke</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="269"/>
        <source>HRV</source>
        <translation>HRV</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="278"/>
        <source>Skin temp. vs usual</source>
        <translation>Ihon lämpö vs. tavallinen</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="278"/>
        <source>Skin temperature</source>
        <translation>Ihon lämpötila</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="284"/>
        <source>Blood oxygen</source>
        <translation>Veren happi</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="290"/>
        <source>Breathing</source>
        <translation>Hengitys</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="297"/>
        <source>Heart rate, %1 ago</source>
        <translation>Syke, %1 sitten</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="298"/>
        <source>Heart rate</source>
        <translation>Syke</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="308"/>
        <source>Oura&apos;s scores</source>
        <translation>Ouran pisteet</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="322"/>
        <source>Readiness</source>
        <translation>Valmius</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="327"/>
        <source>Activity</source>
        <translation>Aktiivisuus</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="337"/>
        <source>%1 steps</source>
        <translation>%1 askelta</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="347"/>
        <source>Today</source>
        <translation>Tänään</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="362"/>
        <source>Heart rate by hour</source>
        <translation>Syke tunneittain</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="375"/>
        <source>About %1 kcal active · %2 min moderate or harder</source>
        <translation>Noin %1 kcal aktiivista · %2 min kohtalaista tai rankempaa</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="390"/>
        <source>Sleep score, recent nights</source>
        <translation>Unipisteet, viime yöt</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="424"/>
        <source>Sleep history</source>
        <translation>Unihistoria</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="434"/>
        <source>Activity history</source>
        <translation>Aktiivisuushistoria</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="444"/>
        <source>Trends</source>
        <translation>Trendit</translation>
    </message>
    <message>
        <location filename="../qml/pages/MainPage.qml" line="458"/>
        <source>%1 · battery %2 % · firmware %3</source>
        <translation>%1 · akku %2 % · laiteohjelmisto %3</translation>
    </message>
</context>
<context>
    <name>NightPage</name>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="22"/>
        <source>Duration</source>
        <translation>Kesto</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="23"/>
        <location filename="../qml/pages/NightPage.qml" line="126"/>
        <source>Efficiency</source>
        <translation>Tehokkuus</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="24"/>
        <source>Falling asleep</source>
        <translation>Nukahtaminen</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="25"/>
        <source>Time awake in the night</source>
        <translation>Hereillä yöllä</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="26"/>
        <location filename="../qml/pages/NightPage.qml" line="146"/>
        <source>Awakenings</source>
        <translation>Heräämiset</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="27"/>
        <source>Deep and REM sleep</source>
        <translation>Syvä ja REM-uni</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="28"/>
        <source>Heart rate and HRV vs usual</source>
        <translation>Syke ja HRV vs. tavallinen</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="33"/>
        <source>Optimal</source>
        <translation>Erinomainen</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="34"/>
        <source>Good</source>
        <translation>Hyvä</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="35"/>
        <source>Fair</source>
        <translation>Kohtalainen</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="36"/>
        <source>Pay attention</source>
        <translation>Kiinnitä huomiota</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="60"/>
        <source>No data for this night</source>
        <translation>Ei tietoja tältä yöltä</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="76"/>
        <source>Oura</source>
        <translation>Oura</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="76"/>
        <source>Estimate</source>
        <translation>Arvio</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="98"/>
        <source>asleep, estimated</source>
        <translation>unessa, arvio</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="98"/>
        <source>asleep</source>
        <translation>unessa</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="104"/>
        <source>%1 in bed</source>
        <translation>%1 sängyssä</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="133"/>
        <source>To fall asleep</source>
        <translation>Nukahtamiseen</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="140"/>
        <source>Awake in the night</source>
        <translation>Hereillä yöllä</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="153"/>
        <source>Lowest heart rate</source>
        <translation>Alin syke</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="160"/>
        <source>HRV</source>
        <translation>HRV</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="167"/>
        <source>Sleep stages</source>
        <translation>Unen vaiheet</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="184"/>
        <source>Estimated from heart rate, HRV and movement: this ring does not stage sleep itself. Awake and asleep are reliable; deep sleep and REM are rough.</source>
        <translation>Arvioitu sykkeestä, HRV:stä ja liikkeestä: tämä sormus ei vaiheista unta itse. Hereillä ja unessa -jaot ovat luotettavia, syvä uni ja REM karkeita.</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="192"/>
        <source>Deep</source>
        <translation>Syvä</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="193"/>
        <source>Light</source>
        <translation>Kevyt</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="194"/>
        <source>REM</source>
        <translation>REM</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="195"/>
        <source>Awake</source>
        <translation>Hereillä</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="204"/>
        <source>The ring has not reported sleep stages for this night. Its own sleep window is shown, and the time asleep is estimated from it.</source>
        <translation>Sormus ei ole raportoinut unen vaiheita tältä yöltä. Näytetään sormuksen oma unijakso, ja nukuttu aika arvioidaan siitä.</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="211"/>
        <source>Heart rate (bpm)</source>
        <translation>Syke (bpm)</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="224"/>
        <source>Heart rate variability (ms)</source>
        <translation>Sykevaihtelu (ms)</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="237"/>
        <source>Breathing (per minute)</source>
        <translation>Hengitys (minuutissa)</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="252"/>
        <source>Skin temperature (°C), %1 vs usual</source>
        <translation>Ihon lämpötila (°C), %1 vs. tavallinen</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="253"/>
        <source>Skin temperature (°C)</source>
        <translation>Ihon lämpötila (°C)</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="267"/>
        <source>Blood oxygen (%)</source>
        <translation>Veren happi (%)</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="283"/>
        <source>Oura&apos;s score: %1</source>
        <translation>Ouran pisteet: %1</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="300"/>
        <source>In brackets: the same contributor computed here with Oura&apos;s own formula from the night&apos;s stages. The difference shows how close that arithmetic is.</source>
        <translation>Sulkeissa: sama osatekijä laskettuna täällä Ouran omalla kaavalla yön unen vaiheista. Ero kertoo, kuinka lähellä tämä laskenta on.</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="310"/>
        <source>This night comes from Oura&apos;s cloud, not from the ring: stages are in five-minute steps.</source>
        <translation>Tämä yö on Ouran pilvestä, ei sormuksesta: vaiheet ovat viiden minuutin jaksoissa.</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="316"/>
        <source>How the score is made</source>
        <translation>Miten pisteet muodostuvat</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="316"/>
        <source>This app&apos;s estimate: how it is made</source>
        <translation>Tämän sovelluksen arvio: miten se muodostuu</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="366"/>
        <source>This score is this app&apos;s own estimate, built from published sleep research (National Sleep Foundation guidelines). Oura computes its score with closed software on the phone, so the two numbers will not match.</source>
        <translation>Nämä pisteet ovat tämän sovelluksen oma arvio, joka perustuu julkaistuun unitutkimukseen (National Sleep Foundationin suositukset). Oura laskee omat pisteensä puhelimessa suljetulla ohjelmistolla, joten luvut eivät täsmää.</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightPage.qml" line="377"/>
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
        <location filename="../qml/pages/NightsPage.qml" line="73"/>
        <source>No nights yet</source>
        <translation>Ei vielä öitä</translation>
    </message>
    <message>
        <location filename="../qml/pages/NightsPage.qml" line="74"/>
        <source>Sync the ring after a night&apos;s sleep</source>
        <translation>Synkronoi sormus yöunen jälkeen</translation>
    </message>
</context>
<context>
    <name>OuraCloudClient</name>
    <message>
        <location filename="../src/cloud/ouraclient.cpp" line="68"/>
        <source>Enter the client id and client secret of your Oura application first.</source>
        <translation>Anna ensin Oura-sovelluksesi client id ja client secret.</translation>
    </message>
    <message>
        <location filename="../src/cloud/ouraclient.cpp" line="73"/>
        <source>Could not listen on port %1 for Oura&apos;s redirect: %2. You can still paste the address the browser ends up on.</source>
        <translation>Porttia %1 ei voitu kuunnella Ouran uudelleenohjausta varten: %2. Voit silti liittää osoitteen, johon selain päätyy.</translation>
    </message>
    <message>
        <location filename="../src/cloud/ouraclient.cpp" line="142"/>
        <source>That address belongs to a different sign-in. Start again.</source>
        <translation>Tuo osoite kuuluu toiseen kirjautumiseen. Aloita alusta.</translation>
    </message>
    <message>
        <location filename="../src/cloud/ouraclient.cpp" line="145"/>
        <source>There is no authorization code in what was entered.</source>
        <translation>Annetussa ei ole valtuutuskoodia.</translation>
    </message>
    <message>
        <location filename="../src/cloud/ouraclient.cpp" line="163"/>
        <source>Oura did not accept the code (HTTP %1): %2</source>
        <translation>Oura ei hyväksynyt koodia (HTTP %1): %2</translation>
    </message>
    <message>
        <location filename="../src/cloud/ouraclient.cpp" line="212"/>
        <source>Oura&apos;s reply did not contain a token (HTTP %1, parse error at byte %2): %3</source>
        <translation>Ouran vastauksessa ei ollut tunnistetta (HTTP %1, jäsennysvirhe tavussa %2): %3</translation>
    </message>
    <message>
        <location filename="../src/cloud/ouraclient.cpp" line="213"/>
        <source>(empty)</source>
        <translation>(tyhjä)</translation>
    </message>
    <message>
        <location filename="../src/cloud/ouraclient.cpp" line="227"/>
        <source>Not connected to Oura. Connect again.</source>
        <translation>Ei yhteyttä Ouraan. Yhdistä uudelleen.</translation>
    </message>
    <message>
        <location filename="../src/cloud/ouraclient.cpp" line="240"/>
        <source>Oura refused to renew the sign-in (HTTP %1). Connect again.</source>
        <translation>Oura kieltäytyi uusimasta kirjautumista (HTTP %1). Yhdistä uudelleen.</translation>
    </message>
    <message>
        <location filename="../src/cloud/ouraclient.cpp" line="280"/>
        <source>Oura did not answer in time. Check the network connection.</source>
        <translation>Oura ei vastannut ajoissa. Tarkista verkkoyhteys.</translation>
    </message>
    <message>
        <location filename="../src/cloud/ouraclient.cpp" line="287"/>
        <source>Could not reach Oura: %1</source>
        <translation>Ouraan ei saatu yhteyttä: %1</translation>
    </message>
    <message>
        <location filename="../src/cloud/ouraclient.cpp" line="310"/>
        <source>Oura&apos;s reply could not be read.</source>
        <translation>Ouran vastausta ei voitu lukea.</translation>
    </message>
    <message>
        <location filename="../src/cloud/ouraclient.cpp" line="330"/>
        <source>Oura answered HTTP %1 for the profile.</source>
        <translation>Oura vastasi HTTP %1 profiilille.</translation>
    </message>
    <message>
        <location filename="../src/cloud/ouraclient.cpp" line="356"/>
        <source>Not connected to Oura.</source>
        <translation>Ei yhteyttä Ouraan.</translation>
    </message>
    <message>
        <location filename="../src/cloud/ouraclient.cpp" line="361"/>
        <source>Fetching %1…</source>
        <translation>Haetaan: %1…</translation>
    </message>
    <message>
        <location filename="../src/cloud/ouraclient.cpp" line="387"/>
        <source>Oura did not accept the sign-in (HTTP 401). Disconnect and connect again.</source>
        <translation>Oura ei hyväksynyt kirjautumista (HTTP 401). Katkaise yhteys ja yhdistä uudelleen.</translation>
    </message>
    <message>
        <location filename="../src/cloud/ouraclient.cpp" line="389"/>
        <location filename="../src/cloud/ouraclient.cpp" line="429"/>
        <source>Oura says too many requests were made. Try again in a few minutes.</source>
        <translation>Oura ilmoittaa liian monesta pyynnöstä. Yritä muutaman minuutin kuluttua.</translation>
    </message>
    <message>
        <location filename="../src/cloud/ouraclient.cpp" line="394"/>
        <source>Oura answered HTTP %1 for %2.</source>
        <translation>Oura vastasi HTTP %1 kohteelle %2.</translation>
    </message>
    <message>
        <location filename="../src/cloud/ouraclient.cpp" line="407"/>
        <source>Fetching heart rate from %1…</source>
        <translation>Haetaan sykettä alkaen %1…</translation>
    </message>
    <message>
        <location filename="../src/cloud/ouraclient.cpp" line="447"/>
        <source>Oura refused: %1 (no membership, or the permission was declined).</source>
        <translation>Oura kieltäytyi: %1 (ei jäsenyyttä tai lupa evättiin).</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../src/cloud/exportimport.cpp" line="31"/>
        <location filename="../src/cloud/exportimport.cpp" line="81"/>
        <source>Reading %1…</source>
        <translation>Luetaan: %1…</translation>
    </message>
    <message>
        <location filename="../src/cloud/exportimport.cpp" line="54"/>
        <location filename="../src/cloud/exportimport.cpp" line="61"/>
        <source>Could not open the file</source>
        <translation>Tiedostoa ei voitu avata</translation>
    </message>
    <message>
        <location filename="../src/cloud/exportimport.cpp" line="64"/>
        <source>Opening the archive…</source>
        <translation>Avataan arkistoa…</translation>
    </message>
    <message>
        <location filename="../src/cloud/exportimport.cpp" line="87"/>
        <source>No Oura data files were found. The export is a ZIP of CSV files such as dailysleep.csv, from membership.ouraring.com/data-export.</source>
        <translation>Oura-datatiedostoja ei löytynyt. Vienti on ZIP, jossa on CSV-tiedostoja kuten dailysleep.csv, osoitteesta membership.ouraring.com/data-export.</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="1435"/>
        <source>Saving %1 documents and %2 heart rate samples…</source>
        <translation>Tallennetaan %1 dokumenttia ja %2 sykenäytettä…</translation>
    </message>
    <message>
        <location filename="../src/controller/appcontroller.cpp" line="1439"/>
        <source>Could not save the import: %1</source>
        <translation>Tuontia ei voitu tallentaa: %1</translation>
    </message>
</context>
<context>
    <name>RingLink</name>
    <message>
        <location filename="../src/ble/ringlink.cpp" line="124"/>
        <source>Pairing timed out. Is the ring close to the phone and not connected to another device?</source>
        <translation>Pariutus aikakatkaistiin. Onko sormus lähellä puhelinta eikä yhdistettynä toiseen laitteeseen?</translation>
    </message>
    <message>
        <location filename="../src/ble/ringlink.cpp" line="130"/>
        <source>The ring refused to pair (%1). A ring that is still set up with the Oura app must be factory reset first.</source>
        <translation>Sormus kieltäytyi pariutumasta (%1). Sormus, joka on yhä otettu käyttöön Oura-sovelluksella, täytyy ensin palauttaa tehdasasetuksiin.</translation>
    </message>
    <message>
        <location filename="../src/ble/ringlink.cpp" line="132"/>
        <source>Pairing failed: %1

If this keeps happening, pair the ring once in Settings &gt; Bluetooth, then set it up here.</source>
        <translation>Pariutus epäonnistui: %1

Jos tämä toistuu, pariuta sormus kerran kohdassa Asetukset &gt; Bluetooth ja ota se sitten käyttöön täällä.</translation>
    </message>
    <message>
        <location filename="../src/ble/ringlink.cpp" line="172"/>
        <source>Could not reach the ring. Is it nearby, charged, and not connected to another phone?</source>
        <translation>Sormukseen ei saatu yhteyttä. Onko se lähellä, ladattu eikä yhdistettynä toiseen puhelimeen?</translation>
    </message>
    <message>
        <location filename="../src/ble/ringlink.cpp" line="176"/>
        <source>Could not connect to the ring: %1</source>
        <translation>Sormukseen yhdistäminen epäonnistui: %1</translation>
    </message>
    <message>
        <location filename="../src/ble/ringlink.cpp" line="182"/>
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
        <location filename="../src/ble/ringlink.cpp" line="328"/>
        <source>The ring is not connected</source>
        <translation>Sormus ei ole yhdistettynä</translation>
    </message>
    <message>
        <location filename="../src/ble/ringlink.cpp" line="341"/>
        <source>Could not write to the ring: %1</source>
        <translation>Sormukseen ei voitu kirjoittaa: %1</translation>
    </message>
    <message>
        <location filename="../src/ble/ringlink.cpp" line="388"/>
        <source>Bluetooth link to the ring was lost</source>
        <translation>Bluetooth-yhteys sormukseen katkesi</translation>
    </message>
</context>
<context>
    <name>RingScanner</name>
    <message>
        <location filename="../src/ble/ringscanner.cpp" line="55"/>
        <source>Could not reach Bluetooth (org.bluez): %1</source>
        <translation>Bluetoothiin ei saatu yhteyttä (org.bluez): %1</translation>
    </message>
    <message>
        <location filename="../src/ble/ringscanner.cpp" line="66"/>
        <location filename="../src/ble/ringscanner.cpp" line="162"/>
        <source>No Bluetooth adapter found</source>
        <translation>Bluetooth-sovitinta ei löytynyt</translation>
    </message>
    <message>
        <location filename="../src/ble/ringscanner.cpp" line="95"/>
        <source>Could not list Bluetooth devices: %1</source>
        <translation>Bluetooth-laitteita ei voitu listata: %1</translation>
    </message>
    <message>
        <location filename="../src/ble/ringscanner.cpp" line="123"/>
        <source>Could not start scanning: %1</source>
        <translation>Skannausta ei voitu aloittaa: %1</translation>
    </message>
</context>
<context>
    <name>ScoreCheckPage</name>
    <message>
        <location filename="../qml/pages/ScoreCheckPage.qml" line="57"/>
        <source>Score check</source>
        <translation>Pisteiden vertailu</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScoreCheckPage.qml" line="64"/>
        <source>Oura&apos;s sleep score against this app&apos;s estimate, and against Oura&apos;s own arithmetic applied to your night (age from Settings). Contributor errors are the mean difference in points. Restfulness needs the ring&apos;s own movement data, so it appears only for nights recorded on the ring, not for nights imported from Oura&apos;s cloud.</source>
        <translation>Ouran unipisteet verrattuna tämän sovelluksen arvioon ja Ouran omaan laskentaan sovellettuna yöhösi (ikä Asetuksista). Osatekijöiden virheet ovat pisteiden keskimääräinen ero. Levollisuus tarvitsee sormuksen oman liiketiedon, joten se näkyy vain sormuksella tallennetuilta öiltä, ei Ouran pilvestä tuoduilta.</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScoreCheckPage.qml" line="88"/>
        <source>Night by night</source>
        <translation>Yö kerrallaan</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScoreCheckPage.qml" line="104"/>
        <source>Oura</source>
        <translation>Oura</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScoreCheckPage.qml" line="111"/>
        <source>formula</source>
        <translation>kaava</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScoreCheckPage.qml" line="118"/>
        <source>estimate</source>
        <translation>arvio</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScoreCheckPage.qml" line="125"/>
        <source>Nothing to compare</source>
        <translation>Ei vertailtavaa</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScoreCheckPage.qml" line="126"/>
        <source>Import your Oura data first</source>
        <translation>Tuo ensin Oura-datasi</translation>
    </message>
</context>
<context>
    <name>SettingsPage</name>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="20"/>
        <source>The ring has been reset. Set it up again from the main page&apos;s menu, or on another phone.</source>
        <translation>Sormus on nollattu. Aseta se uudelleen pääsivun valikosta tai toisessa puhelimessa.</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="32"/>
        <source>Settings</source>
        <translation>Asetukset</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="34"/>
        <source>About you</source>
        <translation>Sinusta</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="42"/>
        <source>Used only for the sleep score&apos;s age bands and for the calorie estimate. It is not sent to the ring and does not leave the phone.</source>
        <translation>Käytetään vain unipisteiden ikäryhmiin ja kalorien arvioon. Sitä ei lähetetä sormukseen eikä se poistu puhelimesta.</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="48"/>
        <source>Birthday</source>
        <translation>Syntymäpäivä</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="51"/>
        <source>Not set</source>
        <translation>Ei asetettu</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="66"/>
        <source>Age</source>
        <translation>Ikä</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="76"/>
        <source>Without a birthday the age is the figure Oura&apos;s profile gave, or 40. The score&apos;s limits step with the age, so set the birthday if you do not use Oura&apos;s cloud.</source>
        <translation>Ilman syntymäpäivää ikä on Ouran profiilin antama luku tai 40. Pisteiden rajat muuttuvat iän mukana, joten aseta syntymäpäivä, jos et käytä Ouran pilveä.</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="82"/>
        <source>Daily activity goal, kcal</source>
        <translation>Päivän aktiivisuustavoite, kcal</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="92"/>
        <source>Weight, kg</source>
        <translation>Paino, kg</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="103"/>
        <source>Oura cloud and data export</source>
        <translation>Oura-pilvi ja datavienti</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="109"/>
        <source>Export and backup</source>
        <translation>Vienti ja varmuuskopio</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="115"/>
        <source>Ring</source>
        <translation>Sormus</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="119"/>
        <source>Name</source>
        <translation>Nimi</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="124"/>
        <source>Serial</source>
        <translation>Sarjanumero</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="129"/>
        <source>Firmware</source>
        <translation>Laiteohjelmisto</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="134"/>
        <source>Battery</source>
        <translation>Akku</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="139"/>
        <source>Last sync</source>
        <translation>Viimeisin synkronointi</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="148"/>
        <source>Back up the key</source>
        <translation>Varmuuskopioi avain</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="155"/>
        <source>Diagnostics</source>
        <translation>Diagnostiikka</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="164"/>
        <source>Forget this ring</source>
        <translation>Unohda tämä sormus</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="165"/>
        <source>Forgetting the ring and its history</source>
        <translation>Unohdetaan sormus ja sen historia</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="176"/>
        <source>Factory reset</source>
        <translation>Tehdasasetukset</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="185"/>
        <source>To move the ring to another phone, or to use it with the Oura app again, reset it. The app first reads what the ring holds, then erases the ring&apos;s key, its Bluetooth pairings and its stored history. What is already on this phone stays. Afterwards the ring can be set up here again, or on another phone; the Oura app will install its own key.</source>
        <translation>Jos haluat siirtää sormuksen toiseen puhelimeen tai käyttää sitä taas Oura-sovelluksella, nollaa se. Sovellus lukee ensin sormuksen sisällön ja poistaa sitten sormuksen avaimen, Bluetooth-parit ja tallennetun historian. Se, mikä on jo tässä puhelimessa, säilyy. Sen jälkeen sormuksen voi asettaa uudelleen täällä tai toisessa puhelimessa; Oura-sovellus asentaa siihen oman avaimensa.</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="196"/>
        <source>Factory reset the ring</source>
        <translation>Nollaa sormus tehdasasetuksille</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="197"/>
        <source>Resetting the ring</source>
        <translation>Nollataan sormusta</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="240"/>
        <source>If the ring could not be reached, nothing was reset: put it on its charger, close to the phone, and try again.</source>
        <translation>Jos sormukseen ei saatu yhteyttä, mitään ei nollattu: laita se laturiinsa lähelle puhelinta ja yritä uudelleen.</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="249"/>
        <source>If the key is lost, or the ring will not connect, the app cannot reset it: lift the ring off its charger, put it back and wait two seconds. Then flip the charger with the ring on it upside down until the light turns blue, upside up until red, upside down until magenta and upside up until yellow. After a few minutes the light blinks blue: the ring is reset, and what it held is gone. Do not finish the Oura app&apos;s set-up afterwards unless you want its key.</source>
        <translation>Jos avain on kadonnut tai sormus ei yhdisty, sovellus ei voi nollata sitä: nosta sormus pois latauslaitteesta, laita se takaisin ja odota kaksi sekuntia. Käännä sitten latauslaite sormus päällä ylösalaisin, kunnes valo muuttuu siniseksi, oikein päin, kunnes punaiseksi, ylösalaisin, kunnes magentaksi, ja oikein päin, kunnes keltaiseksi. Muutaman minuutin kuluttua valo vilkkuu sinisenä: sormus on nollattu ja sen sisältämä tieto on poissa. Älä päätä Oura-sovelluksen käyttöönottoa, ellet halua sen avainta.</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="264"/>
        <source>Forgetting removes the stored history and the ring&apos;s key from this phone. The ring keeps its key, so back it up first if you may want to read the ring again.</source>
        <translation>Unohtaminen poistaa tallennetun historian ja sormuksen avaimen tästä puhelimesta. Sormus säilyttää avaimensa, joten varmuuskopioi se ensin, jos saatat haluta lukea sormuksen uudelleen.</translation>
    </message>
</context>
<context>
    <name>SetupPage</name>
    <message>
        <location filename="../qml/pages/SetupPage.qml" line="59"/>
        <source>Set up ring</source>
        <translation>Ota sormus käyttöön</translation>
    </message>
    <message>
        <location filename="../qml/pages/SetupPage.qml" line="68"/>
        <source>A ring has one owner. To use it here it must be factory reset first: remove it from the Oura app, then reset it (on a Gen 3 or 4 dock: place the ring, wait two seconds, flip the dock upside down, up, down and up again, waiting for the light to turn blue, red, magenta and yellow). Do not finish the Oura app&apos;s set-up afterwards: that installs Oura&apos;s own key and locks this app out.</source>
        <translation>Sormuksella on yksi omistaja. Jotta voit käyttää sitä täällä, se täytyy ensin palauttaa tehdasasetuksiin: poista se Oura-sovelluksesta ja nollaa se (Gen 3 tai 4 -telakassa: aseta sormus telakkaan, odota kaksi sekuntia, käännä telakka ylösalaisin, takaisin, taas ylösalaisin ja takaisin, ja odota joka kerta valon muuttuvan siniseksi, punaiseksi, magentaksi ja keltaiseksi). Älä suorita Oura-sovelluksen käyttöönottoa loppuun sen jälkeen: se asentaa Ouran oman avaimen ja lukitsee tämän sovelluksen ulos.</translation>
    </message>
    <message>
        <location filename="../qml/pages/SetupPage.qml" line="81"/>
        <source>After set-up the Oura app can no longer use this ring until it is reset again.</source>
        <translation>Käyttöönoton jälkeen Oura-sovellus ei voi käyttää tätä sormusta, ennen kuin se nollataan uudelleen.</translation>
    </message>
    <message>
        <location filename="../qml/pages/SetupPage.qml" line="86"/>
        <source>My ring already has a key</source>
        <translation>Sormuksellani on jo avain</translation>
    </message>
    <message>
        <location filename="../qml/pages/SetupPage.qml" line="87"/>
        <source>Use this after reinstalling the app, with the key you saved</source>
        <translation>Käytä tätä sovelluksen uudelleenasennuksen jälkeen tallentamallasi avaimella</translation>
    </message>
    <message>
        <location filename="../qml/pages/SetupPage.qml" line="97"/>
        <source>Key, 32 hexadecimal characters</source>
        <translation>Avain, 32 heksadesimaalimerkkiä</translation>
    </message>
    <message>
        <location filename="../qml/pages/SetupPage.qml" line="107"/>
        <source>Looking for rings…</source>
        <translation>Etsitään sormuksia…</translation>
    </message>
    <message>
        <location filename="../qml/pages/SetupPage.qml" line="107"/>
        <source>Rings nearby</source>
        <translation>Sormukset lähellä</translation>
    </message>
    <message>
        <location filename="../qml/pages/SetupPage.qml" line="117"/>
        <source>None yet. Keep the ring close to the phone and make sure no other phone is connected to it. A factory-reset ring starts advertising once the reset has finished.</source>
        <translation>Ei vielä yhtään. Pidä sormus lähellä puhelinta ja varmista, ettei mikään toinen puhelin ole yhdistettynä siihen. Tehdasasetuksiin palautettu sormus alkaa mainostaa itseään, kun nollaus on valmis.</translation>
    </message>
    <message>
        <location filename="../qml/pages/SetupPage.qml" line="163"/>
        <source>Setting up %1</source>
        <translation>Otetaan käyttöön: %1</translation>
    </message>
    <message>
        <location filename="../qml/pages/SetupPage.qml" line="183"/>
        <source>paired</source>
        <translation>pariutettu</translation>
    </message>
    <message>
        <location filename="../qml/pages/SetupPage.qml" line="184"/>
        <source>this ring</source>
        <translation>tämä sormus</translation>
    </message>
</context>
<context>
    <name>TrendsPage</name>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="10"/>
        <source>Oura sleep score</source>
        <translation>Ouran unipisteet</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="11"/>
        <source>Oura readiness score</source>
        <translation>Ouran valmiuspisteet</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="12"/>
        <source>Oura activity score</source>
        <translation>Ouran aktiivisuuspisteet</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="13"/>
        <source>Steps</source>
        <translation>Askeleet</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="14"/>
        <source>Sleep score (estimate)</source>
        <translation>Unipisteet (arvio)</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="15"/>
        <source>Time asleep</source>
        <translation>Nukuttu aika</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="16"/>
        <source>Sleep efficiency</source>
        <translation>Unen tehokkuus</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="17"/>
        <source>Deep sleep</source>
        <translation>Syvä uni</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="18"/>
        <source>REM sleep</source>
        <translation>REM-uni</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="19"/>
        <source>Lowest heart rate</source>
        <translation>Alin syke</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="20"/>
        <source>HRV</source>
        <translation>HRV</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="21"/>
        <source>Skin temperature vs usual</source>
        <translation>Ihon lämpötila vs. tavallinen</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="22"/>
        <source>Skin temperature</source>
        <translation>Ihon lämpötila</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="23"/>
        <source>Blood oxygen</source>
        <translation>Veren happi</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="24"/>
        <source>Breathing rate</source>
        <translation>Hengitystiheys</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="72"/>
        <source>Trends</source>
        <translation>Trendit</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="75"/>
        <source>Measure</source>
        <translation>Mittaus</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="89"/>
        <source>Period</source>
        <translation>Jakso</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="92"/>
        <source>14 nights</source>
        <translation>14 yötä</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="93"/>
        <source>30 nights</source>
        <translation>30 yötä</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="94"/>
        <source>90 nights</source>
        <translation>90 yötä</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="124"/>
        <source>Lowest</source>
        <translation>Alin</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="130"/>
        <source>Average</source>
        <translation>Keskiarvo</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="136"/>
        <source>Highest</source>
        <translation>Korkein</translation>
    </message>
    <message>
        <location filename="../qml/pages/TrendsPage.qml" line="146"/>
        <source>Nothing to show for this measure yet.</source>
        <translation>Tästä mittauksesta ei ole vielä mitään näytettävää.</translation>
    </message>
</context>
</TS>
