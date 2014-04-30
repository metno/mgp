// --- BEGIN Global variables -----------------------------------
// --- END Global variables -------------------------------------

function updateAddTestResultAction() {
    var appName = currentAppName();
    var versionName = currentVersionName();
    var testName = currentTestName();

    if (appName && versionName && testName) {
        $("#add_test_result_button").attr("enabled", true);
        $("#add_test_result_button").attr("disabled", false);
        $("#add_test_result_label").html(
            "Add test result for " + testName + " (" + appName + " " + versionName + ")");
        $("#add_test_result_label").css("color", "#000000");
    } else {
        $("#add_test_result_button").attr("enabled", false);
        $("#add_test_result_button").attr("disabled", true);
        $("#add_test_result_label").html("Adding test result not possible; missing app, version, or test");
        $("#add_test_result_label").css("color", "#bb0000");
    }

    var reporter = extractArg(queryStringArgs(), "reporter");
    $("#add_test_result_reporter").val(reporter);

    $("#add_test_result_comment").val("");
}

// Returns the current app name.
function currentAppName() {
    return $("#select_apps").val();
}

// Handles selecting an app in the drop-down list.
function selectApp() {
    clearTable("#table_versions");
    clearTable("#table_tests");
    clearTable("#table_test_results");
    $("#test_descr").html("");
    $("#test_result_comment").html("");

    // get versions for the current app
    getVersions(currentAppName());

    updateAddTestResultAction();
}

//--- BEGIN similar functions #1 (1) (refactor!) ----------------------------------------

// Returns the current version (as a jQuery selector of the corresponding <tr> element)
// or undefined if no current version exists.
function currentVersion() {
    var tr = $("#table_versions tr.selectedRow:first");
    return (tr.length > 0) ? tr : undefined;
}

// Returns the current version name (as a string) or undefined if no current version exists.
function currentVersionName() {
    var tr = currentVersion();
    return tr ? tr.find("td:first").text() : undefined;
}

// Returns the first version (as a jQuery selector of the corresponding <tr> element)
// or undefined if no versions exist.
function firstVersion() {
    tr = $("#table_versions tr.tr_versions:nth-child(1)");
    return (tr.length > 0) ? tr : undefined;
}

// Returns the jQuery selector of the <tr> element corresponding to the given version name
// or undefined if no such version exists.
function findVersion(versionName) {
    var td = $("#table_versions td").filter(function() {
        return $(this).text() == versionName;
    });
    return td.length > 0 ? td.parent() : undefined;
}

// Sets given version (if defined) as current.
// tr is the jQuery selector for the corresponding <tr> element.
function setCurrentVersion(tr) {
    if (tr.length == 0) return;
    $("#table_versions tr").removeClass("selectedRow"); // unselect all rows
    tr.addClass("selectedRow"); // select target row
    updateAddTestResultAction();
}

// Handles selecting a version in the version table.
// tr is the jQuery selector for the corresponding <tr> element.
function selectVersion(tr) {
    clearTable("#table_tests");
    clearTable("#table_test_results");
    $("#test_descr").html("");
    $("#test_result_comment").html("");

    setCurrentVersion(tr);
    getTests(currentAppName(), currentVersionName());
}

//--- END similar functions #1 (1) ----------------------------------------

//--- BEGIN similar functions #1 (2) (refactor!) ----------------------------------------

// Returns the current test (as a jQuery selector of the corresponding <tr> element)
// or undefined if no current test exists.
function currentTest() {
    var tr = $("#table_tests tr.selectedRow:first");
    return (tr.length > 0) ? tr : undefined;
}

// Returns the current test name (as a string) or undefined if no current test exists.
function currentTestName() {
    var tr = currentTest();
    return tr ? tr.find("td:first").text() : undefined;
}

// Returns the first test (as a jQuery selector of the corresponding <tr> element)
// or undefined if no tests exist.
function firstTest() {
    var tr = $("#table_tests tr.tr_tests:nth-child(1)");
    if (tr.length == 0)
        return undefined;
    return tr;
}

// Returns the jQuery selector of the <tr> element corresponding to the given test name
// or undefined if no such test exists.
function findTest(testName) {
    var td = $("#table_tests td").filter(function() {
        return $(this).text() == testName;
    });
    return td.length > 0 ? td.parent() : undefined;
}

// Sets given test (if defined) as current.
// tr is the jQuery selector for the corresponding <tr> element.
function setCurrentTest(tr) {
    if (tr.length == 0) return;
    $("#table_tests tr").removeClass("selectedRow"); // unselect all rows
    tr.addClass("selectedRow"); // select target row
    $("#test_descr").html(tr.data("descr")); // update test description
    updateAddTestResultAction();
}

// Handles selecting a test in the test table.
// tr is the jQuery selector for the corresponding <tr> element.
function selectTest(tr) {
    clearTable("#table_test_results");
    $("#test_descr").html("");
    $("#test_result_comment").html("");

    setCurrentTest(tr);
    getTestResults(currentAppName(), currentVersionName(), currentTestName());
}

//--- END similar functions #1 (2) ----------------------------------------

//--- BEGIN similar functions #1 (3) (refactor!) ----------------------------------------

// Returns the first test result or undefined if no test results exist.
function firstTestResult() {
    var tr = $("#table_test_results tr.tr_test_results:nth-child(1)");
    return (tr.length > 0) ? tr : undefined;
}

// Sets given test result (if defined) as current.
// tr is the jQuery selector for the corresponding <tr> element.
function setCurrentTestResult(tr) {
    if (tr.length == 0) return;
    $("#table_test_results tr td").removeClass("selectedColumn"); // unselect all rows
    tr.find("td").addClass("selectedColumn"); // select target row
    tr.find("td:first").removeClass("selectedColumn"); // don't highlight the first column
    $("#test_result_comment").html("<pre>" + tr.data("comment") + "</pre>"); // update test result comment
}

// Handles selecting a test in the test table.
// tr is the jQuery selector for the corresponding <tr> element.
function selectTestResult(tr) {
    $("#test_result_comment").html("");
    setCurrentTestResult(tr);
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
                    var appName = extractArg(queryStringArgs(), "app");
                    if (optionExists("#select_apps", appName))
                            $("#select_apps").val(appName);

                    // get versions for current app
                    getVersions(currentAppName());

                    updateAddTestResultAction();
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
// Sets version and test to versionName and testName respectively if possible.
function getVersions(appName, versionName, testName) {
    statusBase = "getting versions for " + appName + " ...";
    updateStatus(statusBase, true);

    query = "?cmd=get_versions&app=" + appName;
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
                    ntests = data.ntests;
                    ntest_results = data.ntest_results;
                    npassed = data.npassed;
                    nfailed = data.nfailed;
                    ncomments = data.ncomments;
                    html = "";
                    for (i = 0; i < versions.length; ++i) {
                        html += "<tr class=\"tr_versions\">";
                        html += "<td>" + versions[i] + "</td>";
                        html += "<td>" + ntests[i] + "</td>";
                        html += "<td>" + ntest_results[i] + "</td>";
                        html += "<td>" + npassed[i] + "</td>";
                        html += "<td>" + nfailed[i] + "</td>";
                        html += "<td>" + ncomments[i] + "</td>";
                        html += "</tr>";
                    }

                    $("#table_versions > tbody:last").append(html);
                    $("#table_versions").trigger("update");
                    if (html != "") // hm ... why is this test necessary?
                        $("#table_versions").trigger("appendCache");

                    // attempt to set current version from input arg and then from query string
		    if (!versionName) // i.e. input arg undefined
			versionName = extractArg(queryStringArgs(), "version");
                    var tr = findVersion(versionName);
                    if (tr)
                        setCurrentVersion(tr);
                    else
                        // default to first version in the table
                        setCurrentVersion(firstVersion());

                    // get tests for the current app/version
                    getTests(currentAppName(), currentVersionName(), testName);

                    updateAddTestResultAction();
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
// Sets test to testName if possible.
function getTests(appName, versionName, testName) {
    if ((!appName) || (!versionName))
        return;

    statusBase = "getting tests for " + appName + " " + versionName + " ...";
    updateStatus(statusBase, true);

    query = "?cmd=get_tests&app=" + appName + "&version=" + versionName;
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
                    ntest_results = data.ntest_results;
                    npassed = data.npassed;
                    nfailed = data.nfailed;
                    ncomments = data.ncomments;
                    html = "";
                    for (i = 0; i < tests.length; ++i) {
                        html += "<tr class=\"tr_tests\">";
                        html += "<td>" + tests[i] + "</td>";
                        html += "<td>" + ntest_results[i] + "</td>";
                        html += "<td>" + npassed[i] + "</td>";
                        html += "<td>" + nfailed[i] + "</td>";
                        html += "<td>" + ncomments[i] + "</td>";
                        html += "</tr>";
                    }

                    $("#table_tests > tbody:last").append(html);
                    $("#table_tests").trigger("update");
                    if (html != "") // hm ... why is this test necessary?
                        $("#table_tests").trigger("appendCache");

                    // register test descriptions
                    for (i = 0; i < descrs.length; ++i) {
                        var childIndex = i + 1;
                        $("#table_tests tr:nth-child(" + childIndex + ")").data("descr", descrs[i]);
                    }

                    // attempt to set current test from input arge and then from query string
		    if (!testName) // i.e. input arg undefined
			testName = extractArg(queryStringArgs(), "test");
                    tr = findTest(testName);
                    if (tr)
                        setCurrentTest(tr);
                    else
                        // default to first test in the table
                        setCurrentTest(firstTest());

                    // get test results for the current app/version/test
                    getTestResults(currentAppName(), currentVersionName(), currentTestName());
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
    if ((!app) || (!version) || (!test))
	return;

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
                    var ids = data.ids;
                    var timestamps = data.timestamps;
                    var reporters = data.reporters;
                    var ipaddresses = data.ipaddresses;
                    var statuses = data.statuses;
                    var comments = data.comments;

                    html = "";
                    for (i = 0; i < timestamps.length; ++i) {
                        html += "<tr class=\"tr_test_results\">";
                        html += "<td width=\"10px\" style=\"font-weight:bold; color:#f00; cursor:pointer\""
			    + " id=\"test_result_" + ids[i] + "\">X</td>";

                        var date = new Date(timestamps[i] * 1000);
                        var year = date.getFullYear();
                        var month = date.getMonth() + 1;
                        var day = date.getDate();
                        var hour = date.getHours();
                        var min = date.getMinutes();
                        var sec = date.getSeconds();
                        var formattedDate = year + "-" + zeroPad2(month.toString()) + "-"
			    + zeroPad2(day.toString()) + " " + zeroPad2(hour.toString()) + ":"
			    + zeroPad2(min.toString()) + ":" + zeroPad2(sec.toString());
                        html += "<td>" + formattedDate + "</td>";

                        html += "<td>" + reporters[i] + "</td>";

                        var status = statuses[i];
                        var style = "";
                        if (status == "pass")
                            style = " style=\"color:#00aa00\"";
                        else if (status == "fail")
                            style = " style=\"color:#aa0000; font-weight:bold\"";
                        html += "<td" + style + ">" + status + "</td>";

                        html += "<td>" + ipaddresses[i] + "</td>";

                        var comment = comments[i].trim();
                        // OPTION 1: first part of comment itself
                        // if (comment != "")
                        //     comment = comment.substring(0, 10) + " ...";
                        // html += "<td>" + comment + "</td>";
                        // OPTION 2: size of comment
                        //html += "<td>" + comment.length + "</td>";
                        // OPTION 3: whether comment is non-empty
                        html += "<td>" + ((comment.length > 0) ? 1 : 0) + "</td>";

                        html += "</tr>";
                    }

                    $("#table_test_results > tbody:last").append(html);
                    $("#table_test_results").trigger("update");
                    if (html != "") // hm ... why is this test necessary?
                        $("#table_test_results").trigger("appendCache");

                    $('td[id^=test_result_]').hover(
                        function() {
                            $(this).css("background-color", "#f00");
                            $(this).css("color", "#fff");
                        }, function() {
                            $(this).css("background-color", "");
                            $(this).css("color", "#f00");
                        }
                    );

                    // register test result comments
                    for (i = 0; i < comments.length; ++i) {
                        var childIndex = i + 1;
                        $("#table_test_results tr:nth-child(" + childIndex + ")").data("comment", comments[i]);
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

// Adds a test result.
function addTestResult() {
    statusBase = "adding test result ...";
    updateStatus(statusBase, true);

    var reporter = $("#add_test_result_reporter").val().trim();
    if (reporter == "") {
        alert("please set a non-empty reporter");
        return;
    }

    var status = $("#add_test_result_status").val();
    var ipAddress = extractArg(queryStringArgs(), "ipaddress");
    var comment = $("#add_test_result_comment").val().trim();

    query = "?cmd=add_test_result&app=" + currentAppName() + "&version=" + currentVersionName()
        + "&test=" + currentTestName() + "&reporter=" + encodeURIComponent(reporter) + "&status=" + status
	+ "&ipaddress=" + ipAddress + "&comment=" + encodeURIComponent(comment);
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

                    // refresh
                    getVersions(currentAppName(), currentVersionName(), currentTestName());
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

// Removes a test result with a given database ID.
function removeTestResult(id) {
    if (!confirm("Remove test result permanently?"))
	return;

    statusBase = "removing test result ...";
    updateStatus(statusBase, true);

    query = "?cmd=remove_test_result&id=" + id;
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

                    // refresh
                    getVersions(currentAppName(), currentVersionName(), currentTestName());
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
        widthFixed : false,
        showProcessing: true,
        // headerTemplate : '{content} {icon}', // Add icon for jui theme; new in v2.7!

        // widgets: [ 'uitheme', 'zebra', 'stickyHeaders', 'filter' ],
        widgets: [ 'uitheme', 'zebra', 'stickyHeaders' ],

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
            stickyHeaders_attachTo : null,

            // adding zebra striping, using content and default styles
            // - the ui css removes the background from default
            // even and odd class names included for this demo to allow switching themes
            zebra   : ["ui-widget-content even", "ui-state-default odd"],
            // use uitheme widget to apply default jquery ui (jui) class names
            // see the uitheme demo for more details on how to change the class names
            uitheme : 'jui'
        }
    };

    options.widgetOptions.stickyHeaders_attachTo = '.wrapper_versions';
    $("#table_versions").tablesorter(options);
    $(document).on("click", ".tr_versions td", function(e) {
        selectVersion($(e.target).parent());
    });

    options.widgetOptions.stickyHeaders_attachTo = '.wrapper_tests';
    $("#table_tests").tablesorter(options);
    $(document).on("click", ".tr_tests td", function(e) {
        selectTest($(e.target).parent());
    });

    options.widgetOptions.stickyHeaders_attachTo = '.wrapper_test_results';
    $("#table_test_results").tablesorter(options);
    $(document).on("click", ".tr_test_results td", function(e) {
	var idMatches = (" " + $(e.target).attr("id") + " ").match(/\stest_result_(\d+)\s/);
	if (idMatches)
	    removeTestResult(parseInt(idMatches[1]));
	else
            selectTestResult($(e.target).parent());
    });

    getApps();
});
