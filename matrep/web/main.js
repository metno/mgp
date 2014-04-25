// --- BEGIN Global variables -----------------------------------
// --- END Global variables -------------------------------------

// Returns the current app.
function currentApp() {
    return $("#select_apps").val();
}

// Handles selecting an app in the drop-down list.
function selectApp() {
    clearTable("#table_versions");
    clearTable("#table_tests");
    clearTable("#table_test_results");
    $("#test_descr").html("");
    $("#test_result_descr").html("");

    // get versions for the current app
    getVersions(currentApp());
}

//--- BEGIN similar functions #1 (1) (refactor!) ----------------------------------------

// Returns the current version or undefined if no current version exists.
function currentVersion() {
    var currRow = $("#table_versions tr.selectedRow:first");
    if (currRow.length == 0)
        return undefined;
    return $(currRow).find("td:first").text();
}

// Returns the first version or undefined if no versions exist.
function firstVersion() {
    var firstRow = $("#table_versions tr:nth-child(1)");
    if (firstRow.length == 0)
        return undefined;
    return $(firstRow).find("td:first").text();
}

// Returns true iff the version table contains the given version.
function versionExists(version) {
    return $("#table_versions td").filter(function() {
        return $(this).text() == version;
    }).length > 0;
}

// Sets given version (if defined) as current.
function setCurrentVersion(version) {
    if (!version) return;

    // unselect all rows
    $("#table_versions tr").removeClass("selectedRow");

    // select target row (### replace contains() with exact-match alternative! ... TBD)
    $("#table_versions tr:has(td:contains(\"" + version + "\"))").addClass("selectedRow");
}

// Handles selecting a version in the version table.
function selectVersion(version) {
    clearTable("#table_tests");
    clearTable("#table_test_results");
    $("#test_descr").html("");
    $("#test_result_descr").html("");

    setCurrentVersion(version);
    getTests(currentApp(), currentVersion());
}

//--- END similar functions #1 (1) ----------------------------------------

//--- BEGIN similar functions #1 (2) (refactor!) ----------------------------------------

// Returns the current test or undefined if no current test exists.
function currentTest() {
    var currRow = $("#table_tests tr.selectedRow:first");
    if (currRow.length == 0)
        return undefined;
    return $(currRow).find("td:first").text();
}

// Returns the first test or undefined if no tests exist.
function firstTest() {
    var firstRow = $("#table_tests tr:nth-child(1)");
    if (firstRow.length == 0)
        return undefined;
    return $(firstRow).find("td:first").text();
}

// Returns true iff the test table contains the given test.
function testExists(test) {
    return $("#table_tests td").filter(function() {
        return $(this).text() == test;
    }).length > 0;
}

// Sets given test (if defined) as current.
function setCurrentTest(test) {
    if (!test) return;

    // unselect all rows
    $("#table_tests tr").removeClass("selectedRow");

    // select target row (### replace contains() with exact-match alternative! ... TBD)
    $("#table_tests tr:has(td:contains(\"" + test + "\"))").addClass("selectedRow");

    // update test description
    descr = $("#table_tests td").filter(function() {
        return $(this).text() == test;
    }).data("descr");

    $("#test_descr").html(descr);
}

// Handles selecting a test in the test table.
function selectTest(test) {
    clearTable("#table_test_results");
    $("#test_descr").html("");
    $("#test_result_descr").html("");

    setCurrentTest(test);
    getTestResults(currentApp(), currentVersion(), currentTest());
}

//--- END similar functions #1 (2) ----------------------------------------

//--- BEGIN similar functions #1 (3) (refactor!) ----------------------------------------

// Returns the first test result or undefined if no test results exist.
function firstTestResult() {
    var firstRow = $("#table_test_results tr:nth-child(1)");
    if (firstRow.length == 0)
        return undefined;
    return $(firstRow).find("td:first").text(); // ### first td is timestamp, but that is not unique! ... TBD
}

// Sets given test result (if defined) as current.
// ### FOR NOW passing timestamp (first td) as unique ID, but the timestamp is not unique! ... TBD!
function setCurrentTestResult(testResult) {
    if (!testResult) return;

    // unselect all rows
    $("#table_test_results tr").removeClass("selectedRow");

    // select target row (### replace contains() with exact-match alternative! ... TBD)
    $("#table_test_results tr:has(td:contains(\"" + testResult + "\"))").addClass("selectedRow");

    // update test description
    descr = $("#table_test_results td").filter(function() {
        return $(this).text() == testResult;
    }).data("descr");

    $("#test_result_descr").html(descr);
}

// Handles selecting a test in the test table.
// ### FOR NOW passing timestamp (first td) as unique ID, but the timestamp is not unique! ... TBD!
function selectTestResult(testResult) {
    $("#test_result_descr").html("");

    setCurrentTestResult(testResult);
}

//--- END similar functions #1 (3) ----------------------------------------

// Retrieves the available apps.
function getApps() {
    statusBase = "getting apps ...";
    updateStatus(statusBase, true);

    query = "?cmd=get_apps";
    url = "http://" + location.host + "/cgi-bin/matrep.py" + query;

    $.ajax({
        url: url,
        type: "GET",
        dataType: "json",

        success: function(data, textStatus, request) {
            if (request.readyState == 4) {
                if (request.status == 200) {

                    if (data.error != null) {
                        updateStatus(statusBase + " failed: " + data.error, false);
                        return
                    }

                    updateStatus(statusBase + " done", false);
                    updateStatus("", false);

                    // set values in drop-down list
                    $("#select_apps option").remove();
                    $.each(data.apps, function(index, app) {
                        $("<option value=\"" + app + "\">" + app + "</option>").appendTo($("#select_apps"));
                    });

                    // set current app from query string if possible
                    app = extractArg(queryStringArgs(), "app");
                    if (optionExists("#select_apps", app))
                        $("#select_apps").val(app);

                    // get versions for current app
                    getVersions($("#select_apps").val());
                }
            }
        },

        error: function(request, textStatus, errorThrown) {
            descr = errorThrown;
            if (errorThrown == null) {
                descr = "undefined error - is the server down?";
            }
            updateStatus(statusBase + " error: " + descr, false);
        }

        // complete: function(request, textStatus) {
        //     alert("complete; request.status: " + request.status)
        // }

    });

    return false;
}

// Retrieves the available versions for a given app.
function getVersions(app) {
    statusBase = "getting versions for " + app + " ...";
    updateStatus(statusBase, true);

    query = "?cmd=get_versions&app=" + app;
    url = "http://" + location.host + "/cgi-bin/matrep.py" + query;

    $.ajax({
        url: url,
        type: "GET",
        dataType: "json",

        success: function(data, textStatus, request) {
            if (request.readyState == 4) {
                if (request.status == 200) {

                    if (data.error != null) {
                        updateStatus(statusBase + " failed: " + data.error, false);
                        return
                    }

                    updateStatus(statusBase + " done", false);
                    updateStatus("", false);

                    clearTable("#table_versions");

                    // insert new rows
                    versions = data.versions;
                    html = "";
                    for (i = 0; i < versions.length; ++i) {
                        html += "<tr class=\"tr_versions\">";
                        html += "<td>" + versions[i] + "</td>";
                        html += "</tr>";
                    }

                    $("#table_versions > tbody:last").append(html);
                    $("#table_versions").trigger("update");
                    if (html != "") // hm ... why is this test necessary?
                        $("#table_versions").trigger("appendCache");

                    // set current version from query string if possible
                    version = extractArg(queryStringArgs(), "version");
                    if (versionExists(version))
                        setCurrentVersion(version);
                    else
                        // default to first version in the table
                        setCurrentVersion(firstVersion());

                    // get tests for the current app/version, but guard against apps without versions
                    var currVersion = currentVersion();
                    if (currVersion)
                        getTests(currentApp(), currVersion);
                }
            }
        },

        error: function(request, textStatus, errorThrown) {
            descr = errorThrown;
            if (errorThrown == null) {
                descr = "undefined error - is the server down?";
            }
            updateStatus(statusBase + " error: " + descr, false);
        }

        // complete: function(request, textStatus) {
        //     alert("complete; request.status: " + request.status)
        // }

    });

    return false;
}

// Retrieves the available tests for a given app/version.
function getTests(app, version) {
    statusBase = "getting tests for " + app + " " + version + " ...";
    updateStatus(statusBase, true);

    query = "?cmd=get_tests&app=" + app + "&version=" + version;
    url = "http://" + location.host + "/cgi-bin/matrep.py" + query;

    $.ajax({
        url: url,
        type: "GET",
        dataType: "json",

        success: function(data, textStatus, request) {
            if (request.readyState == 4) {
                if (request.status == 200) {

                    if (data.error != null) {
                        updateStatus(statusBase + " failed: " + data.error, false);
                        return
                    }

                    updateStatus(statusBase + " done", false);
                    updateStatus("", false);

                    clearTable("#table_tests");

                    // insert new rows
                    tests = data.tests;
                    descrs = data.descrs;
                    html = "";
                    for (i = 0; i < tests.length; ++i) {
                        html += "<tr class=\"tr_tests\">";
                        html += "<td>" + tests[i] + "</td>";
                        html += "</tr>";
                    }

                    $("#table_tests > tbody:last").append(html);
                    $("#table_tests").trigger("update");
                    if (html != "") // hm ... why is this test necessary?
                        $("#table_tests").trigger("appendCache");

                    // register test descriptions
                    for (i = 0; i < tests.length; ++i) {
                        $("#table_tests td").filter(function() {
                            return $(this).text() == tests[i];
                        }).data("descr", descrs[i]);
                    }

                    // set current test from query string if possible
                    test = extractArg(queryStringArgs(), "test");
                    if (testExists(test))
                        setCurrentTest(test);
                    else
                        // default to first test in the table
                        setCurrentTest(firstTest());

                    // get test results for the current app/version/test, but guard against app/version
                    // without tests
                    var currTest = currentTest();
                    if (currTest)
                        getTestResults(currentApp(), currentVersion(), currTest);
                }
            }
        },

        error: function(request, textStatus, errorThrown) {
            descr = errorThrown;
            if (errorThrown == null) {
                descr = "undefined error - is the server down?";
            }
            updateStatus(statusBase + " error: " + descr, false);
        }

        // complete: function(request, textStatus) {
        //     alert("complete; request.status: " + request.status)
        // }

    });

    return false;
}

// Retrieves the available test results for a given app/version/test.
function getTestResults(app, version, test) {
    statusBase = "getting test results for " + app + " " + version + " " + test + " ...";
    updateStatus(statusBase, true);

    query = "?cmd=get_test_results&app=" + app + "&version=" + version + "&test=" + test;
    url = "http://" + location.host + "/cgi-bin/matrep.py" + query;

    $.ajax({
        url: url,
        type: "GET",
        dataType: "json",

        success: function(data, textStatus, request) {
            if (request.readyState == 4) {
                if (request.status == 200) {

                    if (data.error != null) {
                        updateStatus(statusBase + " failed: " + data.error, false);
                        return
                    }

                    updateStatus(statusBase + " done", false);
                    updateStatus("", false);

                    clearTable("#table_test_results");

                    // insert new rows
                    timestamps = data.timestamps;
                    reporters = data.reporters;
                    ipaddresses = data.ipaddresses;
                    statuses = data.statuses;
                    descrs = data.descrs;
                    html = "";
                    for (i = 0; i < timestamps.length; ++i) {
                        html += "<tr class=\"tr_test_results\">";
                        html += "<td>" + timestamps[i] + "</td>";
                        html += "<td>" + reporters[i] + "</td>";
                        html += "<td>" + statuses[i] + "</td>";
                        html += "<td>" + ipaddresses[i] + "</td>";
                        html += "</tr>";
                    }

                    $("#table_test_results > tbody:last").append(html);
                    $("#table_test_results").trigger("update");
                    if (html != "") // hm ... why is this test necessary?
                        $("#table_test_results").trigger("appendCache");

                    // register test result descriptions
		    // ### WARNING: the timestamp is not unique! ... TBD
                    for (i = 0; i < timestamps.length; ++i) {
                        $("#table_test_results td").filter(function() {
                            return $(this).text() == timestamps[i];
                        }).data("descr", descrs[i]);
                    }

                    // set the first test result in the table as the current test result
                    setCurrentTestResult(firstTestResult());
                }
            }
        },

        error: function(request, textStatus, errorThrown) {
            descr = errorThrown;
            if (errorThrown == null) {
                descr = "undefined error - is the server down?";
            }
            updateStatus(statusBase + " error: " + descr, false);
        }

        // complete: function(request, textStatus) {
        //     alert("complete; request.status: " + request.status)
        // }

    });

    return false;
}

$(document).ready(function() {
    initTablesorter();

    var options = {
        widthFixed : true,
        showProcessing: true,
        // headerTemplate : '{content} {icon}', // Add icon for jui theme; new in v2.7!

        widgets: [ 'uitheme', 'zebra', 'stickyHeaders', 'filter' ],

        widgetOptions: {

            // extra class name added to the sticky header row
            stickyHeaders : '',
            // number or jquery selector targeting the position:fixed element
            stickyHeaders_offset : 0,
            // added to table ID, if it exists
            stickyHeaders_cloneId : '-sticky',
            // trigger "resize" event on headers
            stickyHeaders_addResizeEvent : true,
            // if false and a caption exist, it won't be included in the sticky header
            stickyHeaders_includeCaption : true,
            // The zIndex of the stickyHeaders, allows the user to adjust this to their needs
            stickyHeaders_zIndex : 2,
            // jQuery selector or object to attach sticky header to
            stickyHeaders_attachTo : '.wrapper', // make table scroll within its wrapper

            // adding zebra striping, using content and default styles - the ui css removes the background from default
            // even and odd class names included for this demo to allow switching themes
            zebra   : ["ui-widget-content even", "ui-state-default odd"],
            // use uitheme widget to apply default jquery ui (jui) class names
            // see the uitheme demo for more details on how to change the class names
            uitheme : 'jui'
        }
    };

    $("#table_versions").tablesorter(options);
    $("#table_tests").tablesorter(options);
    $("#table_test_results").tablesorter(options);

    $(document).on("click", ".tr_versions td", function(e) {
        selectVersion(e.target.innerHTML);
    });

    $(document).on("click", ".tr_tests td", function(e) {
        selectTest(e.target.innerHTML);
    });

    $(document).on("click", ".tr_test_results td", function(e) {
        selectTestResult(e.target.innerHTML);
    });

    getApps();
});
