#!/usr/bin/env python3
"""Fills translations/harbour-ourasync-fi.ts from the English extraction base.

lupdate maintains the message list; this holds the Finnish text. Run it after
lupdate (see README) and it reports any source string that has no translation,
so a new qsTr() cannot slip through untranslated unnoticed.
"""
import html
import re
import shutil
import sys

FI = {
"About Oura Sync": "Tietoja: Oura Sync",
"Oura Sync reads an Oura ring directly over Bluetooth LE and shows what it recorded. It is an independent project: it is not made, endorsed or supported by Oura Health, and it is not a medical device.":
    "Oura Sync lukee Oura-sormuksen suoraan Bluetooth LE:n yli ja näyttää, mitä se on tallentanut. Tämä on riippumaton projekti: Oura Health ei ole tehnyt, hyväksynyt eikä tue sitä, eikä se ole lääkinnällinen laite.",
"What you see, and where it comes from": "Mitä näet ja mistä se tulee",
"Measured by the ring: sleep stages and the sleep window, heart rate, heart rate variability, skin temperature, blood oxygen, breathing rate while asleep, and how hard you moved, minute by minute.\n\nEstimated by this app: the sleep score (from published sleep research), the temperature change against your own usual, active calories, and the hourly heart rate bands.\n\nNot available: Oura's own Sleep, Readiness and Activity scores, and step counts. Oura computes those on the phone with closed software, so a ring read directly does not carry them.":
    "Sormuksen mittaamaa: unen vaiheet ja unijakso, syke, sykevaihtelu, ihon lämpötila, veren happikyllästeisyys, hengitystiheys unen aikana sekä liikkeen rasittavuus minuutti minuutilta.\n\nTämän sovelluksen arvioimaa: unipisteet (julkaistun unitutkimuksen pohjalta), lämpötilan poikkeama omasta tavallisesta tasostasi, aktiivisesti kulutetut kalorit ja tuntikohtaiset sykealueet.\n\nEi saatavilla: Ouran omat uni-, valmiusaste- ja aktiivisuuspisteet sekä askeleet. Oura laskee ne puhelimessa suljetulla ohjelmistolla, joten suoraan luettu sormus ei sisällä niitä.",
"Privacy": "Yksityisyys",
"The app has no network permission. The history is kept in a database on this phone and the ring's key in the Sailfish secrets vault.":
    "Sovelluksella ei ole verkkolupaa. Historia on tietokannassa tässä puhelimessa ja sormuksen avain Sailfishin salaisuusholvissa.",
"Credits": "Tekijät ja kiitokset",
"The ring's protocol was reverse engineered by the open_oura project, and the sleep-score method and several decoders come from open_health, both by Thomas Marchand (MIT licence). The dashboards follow the ideas of Cracked Oura. The protocol details were checked against version 7.27.0 of Oura's Android app.":
    "Sormuksen protokollan on selvittänyt open_oura-projekti, ja unipisteiden menetelmä sekä useat dekooderit tulevat open_health-projektista; molemmat ovat Thomas Marchandin (MIT-lisenssi). Näkymät seuraavat Cracked Ouran ideoita. Protokollan yksityiskohdat tarkistettiin Ouran Android-sovelluksen version 7.27.0 koodista.",
"Could not open the local database: %1": "Paikallista tietokantaa ei voitu avata: %1",
"Another operation is still running.": "Toinen toiminto on vielä käynnissä.",
"Done": "Valmis",
"The secrets vault did not answer.": "Salaisuusholvi ei vastannut.",
"Connecting to the ring…": "Yhdistetään sormukseen…",
"The ring's key is not in the secrets vault (%1). Set the ring up again, or enter its key.":
    "Sormuksen avainta ei löydy salaisuusholvista (%1). Ota sormus käyttöön uudelleen tai anna sen avain.",
"not found": "ei löytynyt",
"Authenticating…": "Todennetaan…",
"The ring has been factory reset and has no key. Set it up again.": "Sormus on palautettu tehdasasetuksiin eikä sillä ole avainta. Ota se käyttöön uudelleen.",
"The ring rejected the stored key. Another app has probably set the ring up since; a factory reset and a new set-up are needed.":
    "Sormus hylkäsi tallennetun avaimen. Jokin toinen sovellus on luultavasti ottanut sormuksen käyttöön sen jälkeen; tarvitaan tehdasasetusten palautus ja uusi käyttöönotto.",
"Authentication failed: %1": "Todennus epäonnistui: %1",
"Preparing the ring…": "Valmistellaan sormusta…",
"Could not prepare the ring: %1": "Sormuksen valmistelu epäonnistui: %1",
"Could not set the ring's clock: %1": "Sormuksen kelloa ei voitu asettaa: %1",
"Downloading history…": "Ladataan historiaa…",
"Downloading history: %1 events": "Ladataan historiaa: %1 tapahtumaa",
"History download stopped: %1": "Historian lataus pysähtyi: %1",
"No ring is set up yet.": "Sormusta ei ole vielä otettu käyttöön.",
"Syncing…": "Synkronoidaan…",
"Bluetooth does not know this ring any more. Set it up again.": "Bluetooth ei enää tunne tätä sormusta. Ota se käyttöön uudelleen.",
"Pairing…": "Pariutetaan…",
"Oura ring": "Oura-sormus",
"Installing the ring's key…": "Asennetaan sormuksen avainta…",
"Could not read the system's random source.": "Järjestelmän satunnaislähdettä ei voitu lukea.",
"Could not store the key in the secrets vault: %1": "Avainta ei voitu tallentaa salaisuusholviin: %1",
"The ring would not take a new key: %1\n\nA ring that is still set up with the Oura app must first be factory reset.":
    "Sormus ei ottanut uutta avainta: %1\n\nSormus, joka on yhä otettu käyttöön Oura-sovelluksella, täytyy ensin palauttaa tehdasasetuksiin.",
"A key is 32 hexadecimal characters (16 bytes).": "Avain on 32 heksadesimaalimerkkiä (16 tavua).",
"Storing the key…": "Tallennetaan avainta…",
"Slept %1": "Nukuttu %1",
"No history yet": "Ei vielä historiaa",
"No ring set up": "Sormusta ei otettu käyttöön",
"Active, estimated": "Aktiivinen, arvio",
"Moderate or harder": "Kohtalaista tai rankempaa",
"Light": "Kevyt",
"Lowest heart rate": "Alin syke",
"Highest heart rate": "Korkein syke",
"Average effort": "Keskimääräinen rasitus",
"Heart rate by hour (bpm)": "Syke tunneittain (bpm)",
"Effort by hour (MET)": "Rasitus tunneittain (MET)",
"The ring reports how hard you moved, minute by minute, as a MET value (1 is sitting still). It does not report steps. Active calories are an estimate: each minute above 1 MET counts (MET − 1) × your weight ÷ 60, with the weight from Settings. Heart rate bars show the 5th to 95th percentile of the hour, with a tick at the mean.":
    "Sormus kertoo liikkeen rasittavuuden minuutti minuutilta MET-arvona (1 on paikallaan istumista). Se ei kerro askelia. Aktiiviset kalorit ovat arvio: jokainen 1 METin ylittävä minuutti lasketaan (MET − 1) × painosi ÷ 60, paino Asetuksista. Sykepalkit näyttävät tunnin 5.–95. persentiilin ja keskiarvon pienenä viivana.",
"Activity": "Aktiivisuus",
"%1 days": "%1 päivää",
"Heart rate %1–%2 bpm": "Syke %1–%2 bpm",
"No heart rate": "Ei sykettä",
"%1 min active": "%1 min aktiivinen",
"%1 kcal": "%1 kcal",
"No activity yet": "Ei vielä aktiivisuutta",
"Sync the ring after wearing it for a day": "Synkronoi sormus päivän käytön jälkeen",
"Diagnostics": "Diagnostiikka",
"Stored events by type": "Tallennetut tapahtumat tyypeittäin",
"Awake": "Hereillä",
"REM": "REM",
"Deep": "Syvä",
"Ring is ready": "Sormus on valmis",
"Ring key": "Sormuksen avain",
"The ring is set up and its first history is on its way into the app.": "Sormus on otettu käyttöön ja sen ensimmäinen historia on latautumassa sovellukseen.",
"Your ring is protected by a key that only this phone knows. It is stored in the secure vault. If you lose this phone or its data you will need the key to read the ring again; without it the only way back is a factory reset, which erases the ring's stored history. Save a copy somewhere safe, such as a password manager.":
    "Sormustasi suojaa avain, jonka vain tämä puhelin tuntee. Se on tallessa suojatussa holvissa. Jos menetät tämän puhelimen tai sen tiedot, tarvitset avaimen sormuksen lukemiseen uudelleen; ilman sitä ainoa tie takaisin on tehdasasetusten palautus, joka poistaa sormuksen tallennetun historian. Tallenna kopio turvalliseen paikkaan, esimerkiksi salasanamanageriin.",
"Show key": "Näytä avain",
"The vault did not return a key.": "Holvi ei palauttanut avainta.",
"Copy": "Kopioi",
"Anyone with this key who can pair with the ring can read it. Clear the clipboard after pasting it somewhere safe.":
    "Jokainen, jolla on tämä avain ja joka voi pariutua sormuksen kanssa, voi lukea sen. Tyhjennä leikepöytä, kun olet liittänyt avaimen turvalliseen paikkaan.",
"No data": "Ei tietoja",
"Not synced yet": "Ei vielä synkronoitu",
"Synced just now": "Synkronoitu juuri nyt",
"Synced %1 ago": "Synkronoitu %1 sitten",
"About": "Tietoja",
"Settings": "Asetukset",
"Set up ring": "Ota sormus käyttöön",
"Sync now": "Synkronoi nyt",
"Oura Sync": "Oura Sync",
"Your ring, without the cloud": "Sormuksesi ilman pilveä",
"Oura Sync reads your ring over Bluetooth and shows its sleep, heart rate, HRV, temperature and activity. There is no account, no subscription and no network access: the data stays on this phone.":
    "Oura Sync lukee sormuksesi Bluetoothilla ja näyttää sen unen, sykkeen, sykevaihtelun, lämpötilan ja aktiivisuuden. Ei tiliä, ei tilausta eikä verkkoyhteyttä: tiedot pysyvät tässä puhelimessa.",
"Reading the history…": "Luetaan historiaa…",
"No history yet. Pull down and choose Sync now, with the ring nearby. The first sync downloads everything the ring holds, which takes a few minutes.":
    "Ei vielä historiaa. Vedä alas ja valitse Synkronoi nyt sormus lähellä. Ensimmäinen synkronointi lataa kaiken, mitä sormus sisältää, ja kestää muutaman minuutin.",
"Last night, %1": "Viime yö, %1",
"Sleep": "Uni",
"asleep (estimated)": "unessa (arvio)",
"asleep": "unessa",
"Efficiency %1 %": "Tehokkuus %1 %",
"HRV": "HRV",
"Skin temp. vs usual": "Ihon lämpö vs. tavallinen",
"Skin temperature": "Ihon lämpötila",
"Blood oxygen": "Veren happi",
"Breathing": "Hengitys",
"Heart rate, %1 ago": "Syke, %1 sitten",
"Heart rate": "Syke",
"Today": "Tänään",
"Heart rate by hour": "Syke tunneittain",
"About %1 kcal active · %2 min moderate or harder": "Noin %1 kcal aktiivista · %2 min kohtalaista tai rankempaa",
"Sleep score, recent nights": "Unipisteet, viime yöt",
"Sleep history": "Univiihistoria",
"Activity history": "Aktiivisuushistoria",
"Trends": "Trendit",
"%1 · battery %2 % · firmware %3": "%1 · akku %2 % · laiteohjelmisto %3",
"Duration": "Kesto",
"Efficiency": "Tehokkuus",
"Falling asleep": "Nukahtaminen",
"Time awake in the night": "Hereillä yöllä",
"Awakenings": "Heräämiset",
"Deep and REM sleep": "Syvä ja REM-uni",
"Heart rate and HRV vs usual": "Syke ja HRV vs. tavallinen",
"Optimal": "Erinomainen",
"Good": "Hyvä",
"Fair": "Kohtalainen",
"Pay attention": "Kiinnitä huomiota",
"No data for this night": "Ei tietoja tältä yöltä",
"asleep, estimated": "unessa, arvio",
"%1 in bed": "%1 sängyssä",
"To fall asleep": "Nukahtamiseen",
"Awake in the night": "Hereillä yöllä",
"Sleep stages": "Unen vaiheet",
"The ring has not reported sleep stages for this night. Its own sleep window is shown, and the time asleep is estimated from it.":
    "Sormus ei ole raportoinut unen vaiheita tältä yöltä. Näytetään sormuksen oma unijakso, ja nukuttu aika arvioidaan siitä.",
"Heart rate (bpm)": "Syke (bpm)",
"Heart rate variability (ms)": "Sykevaihtelu (ms)",
"Breathing (per minute)": "Hengitys (minuutissa)",
"Skin temperature (°C), %1 vs usual": "Ihon lämpötila (°C), %1 vs. tavallinen",
"Skin temperature (°C)": "Ihon lämpötila (°C)",
"Blood oxygen (%)": "Veren happi (%)",
"How the score is made": "Miten pisteet muodostuvat",
"This score is this app's own estimate, built from published sleep research (National Sleep Foundation guidelines). Oura computes its score with closed software on the phone, so the two numbers will not match.":
    "Nämä pisteet ovat tämän sovelluksen oma arvio, joka perustuu julkaistuun unitutkimukseen (National Sleep Foundationin suositukset). Oura laskee omat pisteensä puhelimessa suljetulla ohjelmistolla, joten luvut eivät täsmää.",
"The ring's clock could not be tied to the calendar for this night, so its times are placed from the moment of the download and may be off by hours.":
    "Sormuksen kelloa ei saatu sidottua kalenteriin tältä yöltä, joten ajat on sijoitettu latauksen hetken mukaan ja ne voivat olla tuntien päässä.",
"%1 nights": "%1 yötä",
"nap": "päiväunet",
"%1 % efficiency": "%1 % tehokkuus",
"No nights yet": "Ei vielä öitä",
"Sync the ring after a night's sleep": "Synkronoi sormus yöunen jälkeen",
"Pairing timed out. Is the ring close to the phone and not connected to another device?":
    "Pariutus aikakatkaistiin. Onko sormus lähellä puhelinta eikä yhdistettynä toiseen laitteeseen?",
"The ring refused to pair (%1). A ring that is still set up with the Oura app must be factory reset first.":
    "Sormus kieltäytyi pariutumasta (%1). Sormus, joka on yhä otettu käyttöön Oura-sovelluksella, täytyy ensin palauttaa tehdasasetuksiin.",
"Pairing failed: %1\n\nIf this keeps happening, pair the ring once in Settings > Bluetooth, then set it up here.":
    "Pariutus epäonnistui: %1\n\nJos tämä toistuu, pariuta sormus kerran kohdassa Asetukset > Bluetooth ja ota se sitten käyttöön täällä.",
"Could not reach the ring. Is it nearby, charged, and not connected to another phone?":
    "Sormukseen ei saatu yhteyttä. Onko se lähellä, ladattu eikä yhdistettynä toiseen puhelimeen?",
"Could not connect to the ring: %1": "Sormukseen yhdistäminen epäonnistui: %1",
"The ring connected but did not finish Bluetooth service discovery.": "Sormus yhdistyi mutta ei suorittanut Bluetooth-palveluiden etsintää loppuun.",
"The ring refused notifications because the link is not encrypted. Pair the ring first.":
    "Sormus kieltäytyi ilmoituksista, koska yhteys ei ole salattu. Pariuta sormus ensin.",
"Could not subscribe to ring notifications: %1": "Sormuksen ilmoituksia ei voitu tilata: %1",
"Could not list Bluetooth services: %1": "Bluetooth-palveluita ei voitu listata: %1",
"This device does not offer the Oura ring service.": "Tämä laite ei tarjoa Oura-sormuksen palvelua.",
"The ring service is missing its read or write characteristic.": "Sormuspalvelusta puuttuu luku- tai kirjoitusominaisuus.",
"The ring is not connected": "Sormus ei ole yhdistettynä",
"Could not write to the ring: %1": "Sormukseen ei voitu kirjoittaa: %1",
"Bluetooth link to the ring was lost": "Bluetooth-yhteys sormukseen katkesi",
"Could not reach Bluetooth (org.bluez): %1": "Bluetoothiin ei saatu yhteyttä (org.bluez): %1",
"No Bluetooth adapter found": "Bluetooth-sovitinta ei löytynyt",
"Could not list Bluetooth devices: %1": "Bluetooth-laitteita ei voitu listata: %1",
"Could not start scanning: %1": "Skannausta ei voitu aloittaa: %1",
"About you": "Sinusta",
"Used only for the sleep score's age bands and for the calorie estimate. It is not sent to the ring and does not leave the phone.":
    "Käytetään vain unipisteiden ikäryhmiin ja kalorien arvioon. Sitä ei lähetetä sormukseen eikä se poistu puhelimesta.",
"Age, years": "Ikä, vuotta",
"Weight, kg": "Paino, kg",
"Ring": "Sormus",
"Name": "Nimi",
"Serial": "Sarjanumero",
"Firmware": "Laiteohjelmisto",
"Battery": "Akku",
"Last sync": "Viimeisin synkronointi",
"Back up the key": "Varmuuskopioi avain",
"Forget this ring": "Unohda tämä sormus",
"Forgetting the ring and its history": "Unohdetaan sormus ja sen historia",
"Forgetting removes the stored history and the ring's key from this phone. The ring keeps its key, so back it up first if you may want to read the ring again.":
    "Unohtaminen poistaa tallennetun historian ja sormuksen avaimen tästä puhelimesta. Sormus säilyttää avaimensa, joten varmuuskopioi se ensin, jos saatat haluta lukea sormuksen uudelleen.",
"A ring has one owner. To use it here it must be factory reset first: remove it from the Oura app, then reset it (on a Gen 3 or 4 dock: place the ring, wait two seconds, flip the dock upside down, up, down and up again, waiting for the light to turn blue, red, magenta and yellow). Do not finish the Oura app's set-up afterwards: that installs Oura's own key and locks this app out.":
    "Sormuksella on yksi omistaja. Jotta voit käyttää sitä täällä, se täytyy ensin palauttaa tehdasasetuksiin: poista se Oura-sovelluksesta ja nollaa se (Gen 3 tai 4 -telakassa: aseta sormus telakkaan, odota kaksi sekuntia, käännä telakka ylösalaisin, takaisin, taas ylösalaisin ja takaisin, ja odota joka kerta valon muuttuvan siniseksi, punaiseksi, magentaksi ja keltaiseksi). Älä suorita Oura-sovelluksen käyttöönottoa loppuun sen jälkeen: se asentaa Ouran oman avaimen ja lukitsee tämän sovelluksen ulos.",
"After set-up the Oura app can no longer use this ring until it is reset again.": "Käyttöönoton jälkeen Oura-sovellus ei voi käyttää tätä sormusta, ennen kuin se nollataan uudelleen.",
"My ring already has a key": "Sormuksellani on jo avain",
"Use this after reinstalling the app, with the key you saved": "Käytä tätä sovelluksen uudelleenasennuksen jälkeen tallentamallasi avaimella",
"Key, 32 hexadecimal characters": "Avain, 32 heksadesimaalimerkkiä",
"Looking for rings…": "Etsitään sormuksia…",
"Rings nearby": "Sormukset lähellä",
"None yet. Keep the ring close to the phone and make sure no other phone is connected to it. A factory-reset ring starts advertising once the reset has finished.":
    "Ei vielä yhtään. Pidä sormus lähellä puhelinta ja varmista, ettei mikään toinen puhelin ole yhdistettynä siihen. Tehdasasetuksiin palautettu sormus alkaa mainostaa itseään, kun nollaus on valmis.",
"Setting up %1": "Otetaan käyttöön: %1",
"paired": "pariutettu",
"this ring": "tämä sormus",
"Sleep score": "Unipisteet",
"Time asleep": "Nukuttu aika",
"Sleep efficiency": "Unen tehokkuus",
"Deep sleep": "Syvä uni",
"REM sleep": "REM-uni",
"Skin temperature vs usual": "Ihon lämpötila vs. tavallinen",
"Breathing rate": "Hengitystiheys",
"Measure": "Mittaus",
"Period": "Jakso",
"14 nights": "14 yötä",
"30 nights": "30 yötä",
"90 nights": "90 yötä",
"Lowest": "Alin",
"Average": "Keskiarvo",
"Highest": "Korkein",
"Nothing to show for this measure yet.": "Tästä mittauksesta ei ole vielä mitään näytettävää.",
}
FI["Sleep history"] = "Unihistoria"

def main():
    base, out = "translations/harbour-ourasync.ts", "translations/harbour-ourasync-fi.ts"
    shutil.copyfile(base, out)
    text = open(out, encoding="utf-8").read()
    text = text.replace('<TS version="2.1">', '<TS version="2.1" language="fi_FI">', 1) \
        if 'language=' not in text.split('\n')[1] else text
    missing = []

    def fill(match):
        block = match.group(0)
        src = html.unescape(re.search(r"<source>(.*?)</source>", block, re.S).group(1))
        if src not in FI:
            missing.append(src)
            return block
        esc = html.escape(FI[src], quote=False)
        block = re.sub(r"<translation[^>]*>.*?</translation>|<translation[^>]*/>",
                       "<translation>%s</translation>" % esc, block, flags=re.S)
        return block

    text = re.sub(r"<message>.*?</message>", fill, text, flags=re.S)
    open(out, "w", encoding="utf-8").write(text)
    unused = [k for k in FI if k not in {html.unescape(m) for m in re.findall(r"<source>(.*?)</source>", text, re.S)}]
    for m in missing:
        print("UNTRANSLATED:", repr(m))
    for k in unused:
        print("unused entry:", repr(k[:60]))
    return 1 if missing else 0

sys.exit(main())
