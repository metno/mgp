// --- BEGIN Global variables -----------------------------------

var anySpace = / /g;

// --- END Global variables -----------------------------------

// Clears a table.
function clearTable(tableSel) {
    $(tableSel + " tr:gt(0)").remove(); // remove all rows below the header
}

// ### 2 B DOCUMENTED!
function updateStatus(msg, showSpinner) {
    // Note: the "span[id^=status]" selector matches all span elements
    // with an id attribute that begins with "status".
    $("span[id^=status]").text(msg);
    if (showSpinner) {
        $("img[id^=spinner]").css("display", "inline");
        $("img[id^=nospinner]").css("display", "none");
    } else {
        $("img[id^=spinner]").css("display", "none");
        $("img[id^=nospinner]").css("display", "inline");
    }
}

// ### 2 B DOCUMENTED!
function trim(s)
{
    var i;
    var j;
    for (i = 0; i < s.length && s[i] == ' '; ++i) ;
    for (j = s.length - 1; j >= i && s[j] == ' '; --j) ;
    return (i <= j) ? s.substr(i, (j - i) + 1) : "";
}

// ### 2 B DOCUMENTED!
function queryStringArgs() {
    pairs = document.location.search.slice(1).split("&");
    var args = [];
    for (i = 0; i < pairs.length; ++i) {
        pair = pairs[i].split("=");
        if (pair[1] == null) {
            args[pair[0]] = "";
        } else {
            args[pair[0]] = trim(decodeURIComponent(pair[1]));
        }
    }
    return args;
}

// ### 2 B DOCUMENTED!
function extractArg(args, name) {
    arg = args[name];
    if (arg == null) {
        return "";
    }
    return arg;
}

// ### 2 B DOCUMENTED!
function isNonNullNumber(x) {
    return (x != null) && (!isNaN(x));
}

// ### 2 B DOCUMENTED!
// NOTE: This function is currently duplicated elsewhere in Python!
function changeMagnitudeScore(change) {
    var maxChange = 2.0;
    var absChange = (change < 1.0) ? (1.0 / change) : change;
    return (Math.min(absChange, maxChange) - 1.0) / (maxChange - 1.0);
}

// ### 2 B DOCUMENTED!
// NOTE: This function is currently duplicated elsewhere in Python!
function qualityScore(lsd, ni, nz, nc, mdrse) {
    var maxBadSnapshots = 10; // experimental; maybe use max durability score?
    var maxSampleSize = 5;
    var maxLSD = maxBadSnapshots;
    var maxNI = maxBadSnapshots * maxSampleSize;
    var maxNZ = maxBadSnapshots * maxSampleSize;
    var maxNC = maxBadSnapshots;

    var lsdScore = (lsd == -1) ? 0 : Math.min(1, lsd / maxLSD);
    var niScore = Math.min(1, ni / maxNI);
    var nzScore = Math.min(1, nz / maxNZ);
    var ncScore = Math.min(1, nc / maxNC);
    var mdrseScore = (mdrse == -1) ? 0 : (mdrse / 100);

    return (lsdScore + niScore + nzScore + ncScore + mdrseScore) / 5;
}

// Assigns tooltip with text 'text' to jQuery object 'obj'.
function setTooltip(obj, text) {
    obj.attr(
        "title",
        "cssheader=[tooltipHeader1] cssbody=[tooltipBody1] " +
            "delay=[1000] " +
            "fade=[off] " +
            "fadespeed=[0.1] " +
            "offsetx=[15] " +
            "header=[] " +
            "body=[" + text + "]"
    );
}

function emptySHA1() {
    var s = "";
    var size = 40;
    for (var i = 0; i < size; ++i)
        s += "&nbsp;";
    return s;
}

function formatUnixUTCTimestamp(t) {
    if (t < 0) {
	return "invalid timestamp: " + t;
    }

    var d = new Date(t * 1000)
    return "" + d.getFullYear()
	+ "-" + zeroPad2(d.getMonth() + 1)
	+ "-" + zeroPad2(d.getDate().toString())
	+ " " + zeroPad2(d.getHours().toString())
	+ ":" + zeroPad2(d.getMinutes().toString())
	+ ":" + zeroPad2(d.getSeconds().toString());
}

function dateToTimestamp(date) {
    return Math.round(date.getTime() / 1000);
}

// Converts seconds to days.
function secsToDays(secs) {
    var secsInDay = 86400; // 24 * 60 * 60
    return (secs / secsInDay).toFixed(2);
}

// Converts days to seconds.
function daysToSecs(days) {
    var secsInDay = 86400; // 24 * 60 * 60
    return days * secsInDay;
}

// ### 2 B DOCUMENTED!
function zeroPad2(s) {
    return (s.length == 2) ? s : ("0" + s);
}

// ### 2 B DOCUMENTED!
function interpolatedColor(r1, g1, b1, r2, g2, b2, fromValue, toValue, value) {
    // assert fromValue <= toValue
    var frac = Math.max(Math.min(value, toValue), fromValue);
    var r = Math.round((1 - frac) * r1 + frac * r2);
    var g = Math.round((1 - frac) * g1 + frac * g2);
    var b = Math.round((1 - frac) * b1 + frac * b2);
    var color =
        "#" + zeroPad2(r.toString(16)) + zeroPad2(g.toString(16))
        + zeroPad2(b.toString(16));
    return color;
}


// ### 2 B DOCUMENTED!
function scoreColor(score, regressions) {
    return regressions
        ? interpolatedColor(255, 255, 255, 255, 0, 0, 0.0, 1.0, score)
        : interpolatedColor(255, 255, 255, 0, 255, 0, 0.0, 1.0, score);
}


// ### 2 B DOCUMENTED!
function ageColor(secsAgo) {
    var secsInDay = 86400; // 24 * 60 * 60

    var minSecs = 0;
    var gradientSpan = 7; // Color varies from now to gradientSpan days ago
    var maxSecs = gradientSpan * secsInDay;

    var minR = 255;
    var minG = 128;
    var minB = 0;

    var maxR = 228;
    var maxG = 228;
    var maxB = 228;

    var frac = (secsAgo - minSecs) / (maxSecs - minSecs);
    frac = Math.max(Math.min(frac, 1), 0);

    var r = Math.round((1 - frac) * minR + frac * maxR);
    var g = Math.round((1 - frac) * minG + frac * maxG);
    var b = Math.round((1 - frac) * minB + frac * maxB);
    var color =
        "#" + zeroPad2(r.toString(16)) + zeroPad2(g.toString(16))
        + zeroPad2(b.toString(16));
    return color;
}

// ### 2 B DOCUMENTED!
function initTablesorter() {
/*
    // Define a parser that handles mixed standard- and scientific notation
    // and sorts in _descending_ order before any missing values (i.e.
    // use this parser if large values are more interesting than small ones!).
    $.tablesorter.addParser({
        id: "mixed_numeric_desc_before_missing",
        is: function(s) {
            return false; // so this parser is not auto detected
        },
        format: function(s) {
            var f = parseFloat(s);
            if (isNaN(f))
                return "";

            // The following hack ensures that zeros are grouped together
            // when sorting. True zero values should be avoided as they appear
            // to get the same sorting rank as undefined/missing values.
            if (f == 0)
                return Number.MIN_VALUE;

            return f.toFixed(20); // max precision guaranteed by ECMA standard
        },
        type: "numeric"
    });

    // Define a parser that handles mixed standard- and scientific notation and
    // sorts in _ascending_ order before any missing values (i.e.
    // use this parser if small values are more interesting than large ones!).
    $.tablesorter.addParser({
        id: "mixed_numeric_asc_before_missing",
        is: function(s) {
            return false; // so this parser is not auto detected
        },
        format: function(s) {
            var f = parseFloat(s);
            if (isNaN(f))
                return "";

            // The following inversion ensures that values are sorted in
            // ascending order before any missing values.
            // WARNING: This technique assumes non-negative input!
            // Note also that zero-division is not an issue, since "infinity"
            // is handled in a safe way.
            f = 1.0 / f;

            return f.toFixed(20); // max precision guaranteed by ECMA standard
        },
        type: "numeric"
    });
*/
}

function optionExists(selectSel, val) {
  return $(selectSel + " option[value='" + val + "']").length !== 0;
}

/*
// Updates the test case filter table.
// - tableSel is the JQuery selector for the table.
// - testCases is the array of test case names.
// - nc is the maximum number of columns.
function updateTestCaseTable(tableSel, testCases, nc, onclickHandler) {

    // Clear table:
    $(tableSel + " tr").remove();

    // Populate table:

    var nr = Math.ceil(testCases.length / nc); // # of rows
    var c1 = testCases.length % nc; // lowest column index for empty bottom cell

    var html = "";
    for (r = 0; r < nr; ++r) {
        if ((r < (nr - 1)) || (nc > 1))
            html += "<tr>";
        for (c = 0; c < nc; ++c) {
            if ((r == (nr - 1)) && (c1 > 0) && (c >= c1)) {
                if (r > 0)
                    html += "<td></td>"; // Fill in empty bottom cell
            } else {
                var index = -1;
                if (c <= c1)
                    index = r + c * nr;
                else
                    index = r + c1 * nr + (c - c1) * (nr - 1);

                var name = testCases[index];
                html += "<td style=\"white-space: nowrap\">";
                html += "<input type=\"checkbox\" id=\"tc" + index + "\" " +
                    "name=\"" + name + "\"";
                if (!(onclickHandler === undefined))
                    html += "onclick=\"" + onclickHandler + "\"";
                html += "/>";
                html += "<label for=\"tc" + index + "\">" + name +
                    "</label>";
                html += "</td>";
            }
        }
        if ((r < (nr - 1)) || (nc > 1))
            html += "</tr>";
    }

    $(tableSel).append(html);
}

// Toggles the visibilities of two div elements in a mutually exclusive
// fashion (i.e. exactly one of them is visible at any time).
// hiddenSel and shownSel are the JQuery selectors for the div elements that
// represent the hidden and shown state respectively.
function toggleVisibility(hiddenSel, shownSel) {
    var divObj_hidden = $(hiddenSel);
    var divObj_shown = $(shownSel);
    if (divObj_shown.css("display") == "none") {
        divObj_shown.css("display", "block");
        divObj_hidden.css("display", "none");
    } else {
        divObj_shown.css("display", "none");
        divObj_hidden.css("display", "block");
    }
}
*/
